#include <stdint.h>
#include <klib.h>
#include <mm.h>
#include <memlib.h>

static void *mm_malloc_core(size_t size);
static void mm_free_core(void *bp);

static void *extend_heap(size_t size);
static void *coalesce(void *bp);
static size_t get_asize(size_t size);
static void *find_fit(size_t asize);
static void place(void *bp, size_t asize);
static void *init_freelists();
static void set_freeblock(void *bp, size_t size);
static void update_prealloc(void *bp, int pre_alloc);
static char *get_freelist(size_t size);
static void insert_list(void *bp, void *list);
static void *find_insert(size_t size, void *list);
static void remove_list(void *bp);
static void *find_list_by_size(size_t size, void *list);
static int find_heaplist_by_bp(void *bp);
static int find_list_by_bp(void *bp, void *list);
static char *mm_block_log(char *head);
static void mm_block_log_word_by_char(char *ptr);

static int mm_check();
static void mm_check_free_lists();
static void mm_check_free_list(void *list, size_t min_size, size_t max_size);
static void mm_check_heap_list();

#define WSIZE (__WORDSIZE/8)
#define DSIZE (WSIZE << 1)
#define CHUNKSIZE (1 << 5)
#if __WORDSIZE == 32
  #define WUNIT uint32_t
#elif __WORDSIZE == 64
  #define WUNIT uint64_t
#else
  #error __WORDSIZE is not 32 or 64
#endif
/* single word (4) or double word (8) alignment */
#define ALIGNMENT (DSIZE)

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (ALIGNMENT * (((size) +  ALIGNMENT + (ALIGNMENT - 1)) / ALIGNMENT))
#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

#define MIN_BLOCK_SIZE (WSIZE + DSIZE + WSIZE)

#define MAX(x, y) ((x) > (y) ? (x) : (y))
#define MIN(x, y) ((x) > (y) ? (y) : (x))

#define PACK(size, alloc, pre_alloc) ((size) | (alloc) | ((pre_alloc) << 1))

#define GET(p) (*(WUNIT *)(p))
#define PUT(p, val) (*(WUNIT *)(p) = (WUNIT)(val))

#define GET_SIZE(p) ((GET(p)) & (~(0x7)))
#define GET_ALLOC(p) ((GET(p)) & 0x1)
#define GET_PRE_ALLOC(p) ((GET(p) >> 1) & 0x1)

#define HDRP(bp) ((char *)(bp)-WSIZE)
#define FTRP(bp) ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE((char *)(bp)-WSIZE))
#define PREV_BLKP(bp) ((char *)(bp)-GET_SIZE((char *)(bp)-DSIZE))

#define NEXT_FREE_BP_PTR(bp) ((char *)(bp))
#define PREV_FREE_BP_PTR(bp) ((char *)(bp) + WSIZE)

#define NEXT_FREE_BP(bp) ((char *)GET(NEXT_FREE_BP_PTR(bp)))
#define PREV_FREE_BP(bp) ((char *)GET(PREV_FREE_BP_PTR(bp)))

#define IS_END(p) ((GET_SIZE(p) == 0) && (GET_ALLOC(p) == 1))
#define IS_BP_END(bp) (IS_END(HDRP(bp)))

#define IS_CHECK 0
#define IS_LOG 0

// static size_t mm_check_size = 0;
static size_t mm_check_index = 0;

static char *heap_list = NULL;

static char *free_list_32 = NULL;
static char *free_list_64 = NULL;
static char *free_list_128 = NULL;
static char *free_list_256 = NULL;
static char *free_list_512 = NULL;
static char *free_list_1024 = NULL;
static char *free_list_2048 = NULL;
static char *free_list_4096 = NULL;
static char *free_list_other = NULL;

/*
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
  mm_check_index = 0;
  if ((heap_list = mem_sbrk(3 * WSIZE + 9 * WSIZE)) == (void *)-1)
  {
    return -1;
  }

  heap_list = init_freelists();

  PUT(heap_list, PACK(DSIZE, 1, 1));
  PUT(heap_list + 2 * WSIZE, PACK(0, 1, 1));

  heap_list += WSIZE;
  if (extend_heap(CHUNKSIZE) == NULL)
    return -1;
  return 0;
}

// TODO 扩展时是否改考虑合并前者的空块（假如有）
static void *extend_heap(size_t size)
{

  size_t asize;
  char *bp;

  asize = ALIGN(size);
  asize = MAX(asize, MIN_BLOCK_SIZE);
  if ((long)(bp = mem_sbrk(asize)) == -1)
  {
    return NULL;
  }
  set_freeblock(bp, asize);

  PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1, 0));

  return coalesce(bp);
}

static void *coalesce(void *bp)
{
  int next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));

  size_t size = GET_SIZE(HDRP(bp));
  int pre_alloc = GET_PRE_ALLOC(HDRP(bp));

  if ((pre_alloc == 1) && (next_alloc == 1))
  {
    return bp;
  }
  if ((pre_alloc == 1) && (next_alloc == 0))
  {
    remove_list(NEXT_BLKP(bp));
    remove_list(bp);
    size_t next_bp_size = GET_SIZE(HDRP(NEXT_BLKP(bp)));
    size = size + next_bp_size;
    set_freeblock(bp, size);
  }

  if ((pre_alloc == 0) && (next_alloc == 1))
  {
    remove_list(bp);
    remove_list(PREV_BLKP(bp));
    size_t pre_bp_size = GET_SIZE(HDRP(PREV_BLKP(bp)));
    size = size + pre_bp_size;
    set_freeblock(PREV_BLKP(bp), size);
    bp = PREV_BLKP(bp);
  }

  if ((pre_alloc == 0) && (next_alloc == 0))
  {
    remove_list(bp);
    remove_list(NEXT_BLKP(bp));
    remove_list(PREV_BLKP(bp));
    size_t next_bp_size = GET_SIZE(HDRP(NEXT_BLKP(bp)));
    size_t pre_bp_size = GET_SIZE(HDRP(PREV_BLKP(bp)));
    size = size + pre_bp_size + next_bp_size;
    set_freeblock(PREV_BLKP(bp), size);
    bp = PREV_BLKP(bp);
  }

  return bp;
}

/*
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
  if (IS_LOG)
    printf("%d malloc %d\n", mm_check_index++, size);
  return mm_malloc_core(size);
}

static void *mm_malloc_core(size_t size)
{
  if (IS_LOG)
    printf("\t malloc %d\n", size);
  size_t asize;
  size_t extendsize;
  char *bp;

  if (size == 0)
  {
    return NULL;
  }

  asize = get_asize(size);

  if ((bp = find_fit(asize)) != NULL)
  {
    place(bp, asize);
    mm_check();
    return bp;
  }

  extendsize = MAX(asize, CHUNKSIZE);
  if ((bp = extend_heap(extendsize)) == NULL)
  {
    return NULL;
  }
  place(bp, asize);
  mm_check();
  return bp;
}

static void *find_fit(size_t asize)
{
  char *list = get_freelist(asize);
  while (list <= free_list_other)
  {
    char *bp = find_list_by_size(asize, list);
    if (bp != NULL)
    {
      return bp;
    }
    list += WSIZE;
  }
  return NULL;
}

static void place(void *bp, size_t asize)
{
  remove_list(bp);
  WUNIT bp_size = GET_SIZE(HDRP(bp));
  WUNIT bp_prealloc = GET_PRE_ALLOC(HDRP(bp));
  WUNIT rest_size = bp_size - asize;
  if (rest_size < MIN_BLOCK_SIZE)
  {
    PUT(HDRP(bp), PACK(bp_size, 1, bp_prealloc));
    update_prealloc(NEXT_BLKP(bp), 1);
  }
  else
  {
    PUT(HDRP(bp), PACK(asize, 1, bp_prealloc));
    set_freeblock(NEXT_BLKP(bp), rest_size);
    update_prealloc(NEXT_BLKP(bp), 1);
  }
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *bp)
{
  if (IS_LOG)
    printf("%d free\n", mm_check_index++);
  mm_free_core(bp);
}

static void mm_free_core(void *bp)
{
  if (IS_LOG)
    printf("\t free\n");
  size_t size = GET_SIZE(HDRP(bp));
  set_freeblock(bp, size);
  coalesce(bp);
  mm_check();
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
  if (IS_LOG)
    printf("%d realloc %d\n", mm_check_index, size);
  if (ptr == NULL)
  {
    return mm_malloc_core(size);
  }
  if (size == 0)
  {
    mm_free_core(ptr);
    return NULL;
  }
  size_t old_size = GET_SIZE(HDRP(ptr));
  // 计算原块附近能扩展到最大的size
  size_t free_size = old_size;
  char *free_bp = ptr;
  size_t pre_alloc = GET_PRE_ALLOC(HDRP(ptr));
  if (!pre_alloc)
  {
    free_size +=  GET_SIZE(HDRP(PREV_BLKP(ptr)));
    free_bp = PREV_BLKP(ptr);
  }
  size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(ptr)));
  if (!next_alloc)
  {
    free_size += GET_SIZE(HDRP(NEXT_BLKP(ptr)));
  }
  size_t asize = get_asize(size);
  if (asize == free_size)
  {
    // 原块的前驱和后驱都是已配块，而且分配大小没变所以还是原指针不变
    return ptr;
  }
  else if (asize < free_size)
  {
    // 虽然原块附近大致上能放的下，但是这里寻找最优解，看看是不是在空闲链表里有最优解
    char *best_bp = find_fit(asize);
    if (best_bp != NULL)
    {
      size_t best_bp_size = GET_SIZE(HDRP(best_bp));
      if (best_bp_size < free_size)
      {
        // 空闲链表里有最优解
        place(best_bp, asize);
        best_bp_size = GET_SIZE(HDRP(best_bp));
        memset(best_bp, 0, best_bp_size - WSIZE);
        memcpy(best_bp, ptr, MIN(old_size - WSIZE, asize - WSIZE));
        mm_free_core(ptr);
        return best_bp;
      }
      else
      {
        // 空闲链表的不是最优解，所以直接原块附近就地解决
      }
    }
    else
    {
      // 空闲链表里没有最优解，所以直接原块附近就地解决
    }
    if (!pre_alloc)
    {
      remove_list(PREV_BLKP(ptr));
    }
    if (!next_alloc)
    {
      remove_list(NEXT_BLKP(ptr));
    }
    size_t rest_size = free_size - asize;
    if (rest_size < MIN_BLOCK_SIZE)
    {
      asize = free_size;
      rest_size = 0;
    }
    memcpy(free_bp, ptr, MIN(old_size - WSIZE, asize - WSIZE));
    PUT(HDRP(free_bp), PACK(asize, 1, 1));
    if (rest_size > 0)
    {
      set_freeblock(NEXT_BLKP(free_bp), rest_size);
      update_prealloc(NEXT_BLKP(free_bp), 1);
    }
    else
    {
      update_prealloc(NEXT_BLKP(free_bp), 1);
    }
    mm_check();
    return free_bp;
  }
  else
  {
    // 原来的位置肯定是放不下了，无论是左挪还是右挪全都不行，只能考虑调用mm_malloc
    char *newptr = mm_malloc_core(size);
    memset(newptr, 0, asize - WSIZE);
    memcpy(newptr, ptr, MIN(old_size - WSIZE, asize - WSIZE));
    mm_free_core(ptr);
    return newptr;
  }
}

static size_t get_asize(size_t size)
{
  size_t asize;
  if ((size + WSIZE) <= MIN_BLOCK_SIZE)
  {
    asize = MIN_BLOCK_SIZE;
  }
  else
  {
    asize = ALIGN(WSIZE + size);
  }
  return asize;
}

static void *init_freelists()
{
  free_list_32 = heap_list + 0 * WSIZE;
  PUT(free_list_32, 0);
  free_list_64 = heap_list + 1 * WSIZE;
  PUT(free_list_64, 0);
  free_list_128 = heap_list + 2 * WSIZE;
  PUT(free_list_128, 0);
  free_list_256 = heap_list + 3 * WSIZE;
  PUT(free_list_256, 0);
  free_list_512 = heap_list + 4 * WSIZE;
  PUT(free_list_512, 0);
  free_list_1024 = heap_list + 5 * WSIZE;
  PUT(free_list_1024, 0);
  free_list_2048 = heap_list + 6 * WSIZE;
  PUT(free_list_2048, 0);
  free_list_4096 = heap_list + 7 * WSIZE;
  PUT(free_list_4096, 0);
  free_list_other = heap_list + 8 * WSIZE;
  PUT(free_list_other, 0);
  return heap_list + 9 * WSIZE;
}

static void set_freeblock(void *bp, size_t size)
{
  assert(size >= MIN_BLOCK_SIZE);
  int pre_alloc = GET_PRE_ALLOC(HDRP(bp));
  PUT(HDRP(bp), PACK(size, 0, pre_alloc));
  PUT(FTRP(bp), PACK(size, 0, 0));

  update_prealloc(NEXT_BLKP(bp), 0);

  char *freelist = get_freelist(size);
  insert_list(bp, freelist);
}

static void update_prealloc(void *bp, int pre_alloc)
{
  int alloc = GET_ALLOC(HDRP(bp));
  int size = GET_SIZE(HDRP(bp));
  PUT(HDRP(bp), PACK(size, alloc, pre_alloc));
}

/*
 * get_freelist - 寻找对应的空闲链表
 */
static char *get_freelist(size_t size)
{
  if (size <= 32)
  {
    return free_list_32;
  }
  else if (size <= 64)
  {
    return free_list_64;
  }
  else if (size <= 128)
  {
    return free_list_128;
  }
  else if (size <= 256)
  {
    return free_list_256;
  }
  else if (size <= 512)
  {
    return free_list_512;
  }
  else if (size <= 1024)
  {
    return free_list_1024;
  }
  else if (size <= 2048)
  {
    return free_list_2048;
  }
  else if (size <= 4096)
  {
    return free_list_4096;
  }
  else
  {
    return free_list_other;
  }
}

/*
 * insert_list - 将bp插入到空闲链表里
 */
static void insert_list(void *bp, void *list)
{
  size_t bp_size = GET_SIZE(HDRP(bp));
  char *pre_bp = find_insert(bp_size, list);
  char *next_bp = NEXT_FREE_BP(pre_bp);

  PUT(NEXT_FREE_BP_PTR(pre_bp), bp);
  PUT(PREV_FREE_BP_PTR(bp), pre_bp);
  PUT(NEXT_FREE_BP_PTR(bp), next_bp);
  if (next_bp != NULL)
  {
    PUT(PREV_FREE_BP_PTR(next_bp), bp);
  }
}

/*
 * find_insert - 在空闲链表里根据size大小寻找合适的插入节点，返回理论上的前驱节点
 */
static void *find_insert(size_t size, void *list)
{
  char *list_current_bp = (char *)list;
  char *list_next_bp;
  while ((list_next_bp = NEXT_FREE_BP(list_current_bp)) != NULL)
  {
    size_t next_size = GET_SIZE(HDRP(list_next_bp));
    if (size <= next_size)
    {
      return list_current_bp;
    }
    list_current_bp = NEXT_FREE_BP(list_current_bp);
  }
  return list_current_bp;
}

/*
 * remove_list - 将bp从所在空闲链表里删除
 */
static void remove_list(void *bp)
{
  char *next_bp = NEXT_FREE_BP(bp);
  char *pre_bp = PREV_FREE_BP(bp);

  PUT(NEXT_FREE_BP_PTR(pre_bp), next_bp);
  if (next_bp != NULL)
  {
    PUT(PREV_FREE_BP_PTR(next_bp), pre_bp);
  }

  PUT(NEXT_FREE_BP_PTR(bp), NULL);
  PUT(PREV_FREE_BP_PTR(bp), NULL);
}

/*
 * find_list_by_size - 在空闲链表里寻找合适的节点，返回符合最小是size大小的bp指针
 */
static void *find_list_by_size(size_t size, void *list)
{

  char *list_current_bp = (char *)list;
  char *list_next_bp;
  while ((list_next_bp = NEXT_FREE_BP(list_current_bp)) != NULL)
  {
    size_t bp_size = GET_SIZE(HDRP(list_next_bp));
    if (bp_size >= size)
    {
      return list_next_bp;
    }
    list_current_bp = NEXT_FREE_BP(list_current_bp);
  }
  return NULL;
}

/*
 * find_list_by_bp - 在空闲链表里寻找合适的节点，返回地址是bp参数的bp指针
 */
static int find_list_by_bp(void *bp, void *list)
{

  char *list_current_bp = (char *)list;
  char *list_next_bp;
  while ((list_next_bp = NEXT_FREE_BP(list_current_bp)) != NULL)
  {
    if (list_next_bp == bp)
    {
      return 1;
    }
    list_current_bp = NEXT_FREE_BP(list_current_bp);
  }
  return 0;
}

/*
 * find_heaplist_by_bp - 在堆链表里寻找合适的节点，返回地址是bp参数的bp指针
 */
static int find_heaplist_by_bp(void *bp)
{

  char *list_current_bp = (char *)heap_list;
  while (!IS_BP_END(list_current_bp))
  {
    if (list_current_bp == bp)
    {
      return 1;
    }
    list_current_bp = NEXT_BLKP(list_current_bp);
  }
  return 0;
}

void mm_log()
{
  char *word = heap_list - WSIZE;
  while (!IS_END(word))
  {
    word = mm_block_log(word);
  }
  WUNIT size = GET_SIZE(word);
  WUNIT alloc = GET_ALLOC(word);
  WUNIT pre_alloc = GET_PRE_ALLOC(word);
  printf("[%2d/%d/%d]\n", size, alloc, pre_alloc);
}

static char *mm_block_log(char *head)
{
  WUNIT size = GET_SIZE(head);
  WUNIT alloc = GET_ALLOC(head);
  WUNIT pre_alloc = GET_PRE_ALLOC(head);
  WUNIT word_size = size / WSIZE;
  if (!alloc)
  {
    for (size_t i = 0; i < word_size; i++)
    {
      char *block = head + i * WSIZE;
      if ((i == 0) || (i == word_size - 1))
      {
        WUNIT size = GET_SIZE(block);
        WUNIT alloc = GET_ALLOC(block);
        WUNIT pre_alloc = GET_PRE_ALLOC(block);
        printf("[%2d/%d/%d]", size, alloc, pre_alloc);
      }
      else if ((i == 1) || (i == 2))
      {
        printf("[ptr_]" /*, (char *)GET(block)*/);
      }
      else
      {
        mm_block_log_word_by_char(block);
      }
    }
  }
  else
  {
    for (size_t i = 0; i < word_size; i++)
    {
      char *block = head + i * WSIZE;
      if ((i == 0))
      {
        WUNIT size = GET_SIZE(block);
        WUNIT alloc = GET_ALLOC(block);
        printf("[%2d/%d/%d]", size, alloc, pre_alloc);
      }
      else
      {
        mm_block_log_word_by_char(block);
      }
    }
  }
  return head + size;
}

static void mm_block_log_word_by_char(char *ptr)
{
  printf("[");
  for (size_t i = 0; i < WSIZE; i++)
  {
    char *c = ptr + i;
    printf("%c", ((*c == 0) ? '_' : *c));
  }
  printf("]");
}

/*
 * mm_check - A heap checker that scans the heap and checks it for consistency
 */
static int mm_check()
{
  if (IS_CHECK)
  {
    mm_check_free_lists();
    mm_check_heap_list();
  }
  return 0;
}

/*
 * mm_check_free_lists - 检查所有空闲链表
 */
static void mm_check_free_lists()
{
  mm_check_free_list(free_list_32, 0, 32);
  mm_check_free_list(free_list_64, 32, 64);
  mm_check_free_list(free_list_128, 64, 128);
  mm_check_free_list(free_list_256, 128, 256);
  mm_check_free_list(free_list_512, 256, 512);
  mm_check_free_list(free_list_1024, 512, 1024);
  mm_check_free_list(free_list_2048, 1024, 2048);
  mm_check_free_list(free_list_4096, 2048, 4096);
  mm_check_free_list(free_list_other, 4096, SIZE_MAX);
}

/*
 * mm_check_free_list - 检查空闲链表
 * 空闲链表
 * - 是否所有的块都是空闲的
 * - 是否所有的块都符合空闲块的设计，头，前空闲块指针，尾
 * - 是否都是按照小到大排序
 * - 是否都符合大小标准
 * - 是否都在堆上
 */
static void mm_check_free_list(void *list, size_t min_size, size_t max_size)
{
  size_t size = 0;
  char *list_current_bp = (char *)list;
  char *list_next_bp;
  while ((list_next_bp = NEXT_FREE_BP(list_current_bp)) != NULL)
  {
    size_t next_size = GET_SIZE(HDRP(list_next_bp));
    size_t next_alloc = GET_ALLOC(HDRP(list_next_bp));
    char *next_prefreebp = PREV_FREE_BP(list_next_bp);

    size_t foot_next_size = GET_SIZE(FTRP(list_next_bp));
    size_t foot_next_alloc = GET_ALLOC(FTRP(list_next_bp));
    size_t foot_next_prealloc = GET_PRE_ALLOC(FTRP(list_next_bp));

    // 是否所有的块都是空闲的
    assert(next_alloc == 0);
    // 是否所有的块都符合空闲块的设计，头，前空闲块指针，尾
    assert((next_alloc == foot_next_alloc) && (next_size == foot_next_size) && (foot_next_prealloc == 0));
    assert(next_prefreebp == list_current_bp);
    // 是否都是按照小到大排序
    assert(size <= next_size);
    // 是否都符合大小标准
    assert((next_size > min_size) && (next_size <= max_size));
    // 是否都在堆上
    assert(find_heaplist_by_bp(list_next_bp));

    size = next_size;
    list_current_bp = NEXT_FREE_BP(list_current_bp);
  }
}

/*
 * mm_check_heap_list - 检查堆链表
 * 正式链表
 * - 是否有空闲块没有合并
 * - 遍历到的空闲块是否存在于空闲链表
 * - 检查所有块的pre_alloc是否符合条件
 * - 检查所有的块是否符合最小块的大小
 */
static void mm_check_heap_list()
{
  char *current_bp = (char *)heap_list;
  char *next_bp;
  size_t i = 0;
  while (!IS_BP_END(current_bp))
  {
    next_bp = NEXT_BLKP(current_bp);

    size_t current_size = GET_SIZE(HDRP(current_bp));
    size_t current_alloc = GET_ALLOC(HDRP(current_bp));

    size_t next_alloc = GET_ALLOC(HDRP(next_bp));
    size_t next_prealloc = GET_PRE_ALLOC(HDRP(next_bp));

    // 是否有空闲块没有合并
    assert(!((current_alloc == 0) && (next_alloc == 0)));
    // 遍历到的空闲块是否存在于空闲链表
    if (!current_alloc)
    {
      assert(find_list_by_bp(current_bp, (void *)get_freelist(current_size)));
    }
    // 检查所有块的pre_alloc是否符合条件
    assert(current_alloc == next_prealloc);
    if (i)
    {
      // 检查所有的块是否符合最小块的大小
      assert(current_size >= MIN_BLOCK_SIZE);
    }

    current_bp = NEXT_BLKP(current_bp);
    i++;
  }
}

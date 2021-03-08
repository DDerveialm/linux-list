#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "list.h"

#include "common.h"

static uint16_t values[256];

static void list_qsort(struct list_head *head)
{
    struct list_head list_less, list_greater;
    struct listitem *pivot;
    struct listitem *item = NULL, *is = NULL;

    if (list_empty(head) || list_is_singular(head))
        return;

    INIT_LIST_HEAD(&list_less);
    INIT_LIST_HEAD(&list_greater);

    pivot = list_first_entry(head, struct listitem, list);
    list_del(&pivot->list);

    list_for_each_entry_safe (item, is, head, list) {
        if (cmpint(&item->i, &pivot->i) < 0)
            list_move_tail(&item->list, &list_less);
        else
            list_move(&item->list, &list_greater);
    }

    list_qsort(&list_less);
    list_qsort(&list_greater);

    list_add(&pivot->list, head);
    list_splice(&list_less, head);
    list_splice_tail(&list_greater, head);
}

static void list_qsort_nr(struct list_head *head)
{
#define MAX_LEVELS 64
    struct list_head *sl[MAX_LEVELS], *sr[MAX_LEVELS];
    int i = 0;
    sl[0] = head, sr[0] = head;
    while (i >= 0) {
        struct list_head *L = sl[i], *R = sr[i--];
        if (L->next == R)
            continue;
        struct list_head *pivot = L->next, *node = pivot;
        int cl = 0, cr = 0;
        while (node->next != R) {
            if (cmpint(&list_entry(node->next, struct listitem, list)->i,
                       &list_entry(pivot, struct listitem, list)->i) < 0)
                list_move(node->next, L), ++cl;
            else
                node = node->next, ++cr;
        }
        if (cl >= cr) {
            sl[++i] = L, sr[i] = pivot;
            sl[++i] = pivot, sr[i] = R;
        } else {
            sl[++i] = pivot, sr[i] = R;
            sl[++i] = L, sr[i] = pivot;
        }
    }
}

int main(void)
{
    struct list_head testlist;
    struct listitem *item, *is = NULL;
    size_t i;

    random_shuffle_array(values, (uint16_t) ARRAY_SIZE(values));

    INIT_LIST_HEAD(&testlist);

    assert(list_empty(&testlist));

    for (i = 0; i < ARRAY_SIZE(values); i++) {
        item = (struct listitem *) malloc(sizeof(*item));
        assert(item);
        item->i = values[i];
        list_add_tail(&item->list, &testlist);
    }

    assert(!list_empty(&testlist));

    qsort(values, ARRAY_SIZE(values), sizeof(values[0]), cmpint);
    list_qsort_nr(&testlist);

    i = 0;
    list_for_each_entry_safe (item, is, &testlist, list) {
        assert(item->i == values[i]);
        list_del(&item->list);
        free(item);
        i++;
    }

    assert(i == ARRAY_SIZE(values));
    assert(list_empty(&testlist));

    return 0;
}

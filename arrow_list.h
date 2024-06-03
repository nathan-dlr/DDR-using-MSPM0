#ifndef ARROW_LIST_H_
#define ARROW_LIST_H_

struct arrow_list {
    Arrows arr[12];
    Arrows temp[12];
    uint8_t num_arrows = 0;
};
typedef struct arrow_list arrow_list_t;



#endif /* ARROW_LIST_H_ */

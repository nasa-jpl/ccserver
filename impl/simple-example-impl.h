#ifndef __SIMPLE_EXAMPLE_IMPL_H__
#define __SIMPLE_EXAMPLE_IMPL_H__

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <coreconfTypes.h>
#define read_simpleExample_dataStore read_1001
#define write_simpleExample_dataStore write_1001
CoreconfValueT* read_simpleExample_dataStore(void);
#define read_simpleExample_dataStore_dataStoreValue read_1002
#define write_simpleExample_dataStore_dataStoreValue write_1002
uint64_t read_simpleExample_dataStore_dataStoreValue(void);
#define read_simpleExample_dataStore_itemList read_1003
#define write_simpleExample_dataStore_itemList write_1003
CoreconfValueT* read_simpleExample_dataStore_itemList(uint64_t dataStore_itemList_index);
#define read_dataStore_itemList_subItemList read_1005
#define write_dataStore_itemList_subItemList write_1005
CoreconfValueT* read_dataStore_itemList_subItemList(uint64_t itemList_subItemList_index);
#define read_itemList_subItemList_subValue read_1007
#define write_itemList_subItemList_subValue write_1007
int64_t read_itemList_subItemList_subValue(void);
#define read_dataStore_itemList_value read_1008
#define write_dataStore_itemList_value write_1008
int64_t read_dataStore_itemList_value(void);


// User-facing read/write function prototypes
// Implement these functions in your code

#endif

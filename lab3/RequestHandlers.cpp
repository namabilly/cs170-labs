#include "Request.h"
#include "EStore.h"
#include <cstdio>
/*
 * ------------------------------------------------------------------
 * add_item_handler --
 *
 *      Handle an AddItemReq.
 *
 *      Delete the request object when done.
 *
 * Results:
 *      None.
 *
 * ------------------------------------------------------------------
 */
void 
add_item_handler(void *args)
{
    // TODO: Your code here.
    AddItemReq* req = (AddItemReq*) args;
    req->store->addItem(req->item_id, req->quantity, req->price, req->discount);
    printf("Handling AddItemReq: item_id - %d, quantity - %d, price - $%.2f, "
    "discount - %.2f\n", req->item_id, req->quantity, req->price, req->discount);
    delete req;
}

/*
 * ------------------------------------------------------------------
 * remove_item_handler --
 *
 *      Handle a RemoveItemReq.
 *
 *      Delete the request object when done.
 *
 * Results:
 *      None.
 *
 * ------------------------------------------------------------------
 */
void 
remove_item_handler(void *args)
{
    // TODO: Your code here.
    RemoveItemReq* req = (RemoveItemReq*) args;
    req->store->removeItem(req->item_id);
    printf("Handling RemoveItemReq: item_id - %d\n", req->item_id);
    delete req;
}

/*
 * ------------------------------------------------------------------
 * add_stock_handler --
 *
 *      Handle an AddStockReq.
 *
 *      Delete the request object when done.
 *
 * Results:
 *      None.
 *
 * ------------------------------------------------------------------
 */
void 
add_stock_handler(void *args)
{
    // TODO: Your code here.
    AddStockReq* req = (AddStockReq*) args;
    req->store->addStock(req->item_id, req->additional_stock);
    printf("Handling AddStockReq: item_id - %d, additional_stock - %d\n", 
    req->item_id, req->additional_stock);
    delete req;

}


/*
 * ------------------------------------------------------------------
 * change_item_price_handler --
 *
 *      Handle a ChangeItemPriceReq.
 *
 *      Delete the request object when done.
 *
 * Results:
 *      None.
 *
 * ------------------------------------------------------------------
 */
void 
change_item_price_handler(void *args)
{
    // TODO: Your code here.
    ChangeItemPriceReq* req = (ChangeItemPriceReq*) args;
    req->store->priceItem(req->item_id, req->new_price);
    printf("Handling ChangeItemPriceReq: item_id - %d, new_price - $%.2f\n",
    req->item_id, req->new_price);
    delete req;

}

/*
 * ------------------------------------------------------------------
 * change_item_discount_handler --
 *
 *      Handle a ChangeItemDiscountReq.
 *
 *      Delete the request object when done.
 *
 * Results:
 *      None.
 *
 * ------------------------------------------------------------------
 */
void 
change_item_discount_handler(void *args)
{
    // TODO: Your code here.
    ChangeItemDiscountReq* req = (ChangeItemDiscountReq*) args;
    req->store->discountItem(req->item_id, req->new_discount);
    printf("Handling ChangeItemDiscountReq: item_id - %d, new_discount - %.2f\n",
    req->item_id, req->new_discount);
    delete req;
}

/*
 * ------------------------------------------------------------------
 * set_shipping_cost_handler --
 *
 *      Handle a SetShippingCostReq.
 *
 *      Delete the request object when done.
 *
 * Results:
 *      None.
 *
 * ------------------------------------------------------------------
 */
void 
set_shipping_cost_handler(void *args)
{
    // TODO: Your code here.
    SetShippingCostReq* req = (SetShippingCostReq*) args;
    req->store->setShippingCost(req->new_cost);
    printf("Handling SetShippingCostReq: new_cost - $%.2f\n", req->new_cost);
    delete req;
    
}

/*
 * ------------------------------------------------------------------
 * set_store_discount_handler --
 *
 *      Handle a SetStoreDiscountReq.
 *
 *      Delete the request object when done.
 *
 * Results:
 *      None.
 *
 * ------------------------------------------------------------------
 */
void
set_store_discount_handler(void *args)
{
    // TODO: Your code here.
    SetStoreDiscountReq* req = (SetStoreDiscountReq*) args;
    req->store->setStoreDiscount(req->new_discount);
    printf("Handling SetStoreDiscountReq: new_discount - %.2f\n", req->new_discount);
    delete req;

}

/*
 * ------------------------------------------------------------------
 * buy_item_handler --
 *
 *      Handle a BuyItemReq.
 *
 *      Delete the request object when done.
 *
 * Results:
 *      None.
 *
 * ------------------------------------------------------------------
 */
void
buy_item_handler(void *args)
{
    // TODO: Your code here.
    BuyItemReq* req = (BuyItemReq*) args;
    req->store->buyItem(req->item_id, req->budget);
    printf("Handling BuyItemReq: item_id - %d, budget - $%.2f\n",
    req->item_id, req->budget);
    delete req;

}

/*
 * ------------------------------------------------------------------
 * buy_many_items_handler --
 *
 *      Handle a BuyManyItemsReq.
 *
 *      Delete the request object when done.
 *
 * Results:
 *      None.
 *
 * ------------------------------------------------------------------
 */
void
buy_many_items_handler(void *args)
{
    // TODO: Your code here.
    BuyManyItemsReq* req = (BuyManyItemsReq*) args;
    req->store->buyManyItems(&req->item_ids, req->budget);
    printf("Handling BuyManyItemsReq: \n");
    delete req;

}

/*
 * ------------------------------------------------------------------
 * stop_handler --
 *
 *      The thread should exit.
 *
 * Results:
 *      None.
 *
 * ------------------------------------------------------------------
 */
void 
stop_handler(void* args)
{
    // TODO: Your code here.
    printf("Handling StopReq: Quitting\n");
    sthread_exit();
}


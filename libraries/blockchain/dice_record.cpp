#include <bts/blockchain/dice_record.hpp>

namespace bts { namespace blockchain {
    bool dice_record::is_null()const
    {
        return amount == 0;
    }

    dice_record dice_record::make_null()const
    {
        dice_record cpy(*this);
        cpy.amount = 0;
        return cpy;
    }

}} // bts::blockchain

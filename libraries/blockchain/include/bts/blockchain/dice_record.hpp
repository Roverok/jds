#pragma once
#include <bts/blockchain/types.hpp>
#include <bts/blockchain/withdraw_types.hpp>
#include <bts/blockchain/transaction.hpp>
namespace bts { namespace blockchain {
	typedef transaction_id_type                dice_id_type;

    struct dice_record
    {
        dice_record():jackpot_block_num(0){}

        bool is_null()const;

        dice_record make_null()const;

        dice_id_type        id = dice_id_type();
        address             owner;
        share_type          amount;
        double            payouts;
        bool roll_high;
        uint32_t	jackpot_block_num;
    };

    struct jackpot_transaction
    {
        jackpot_transaction(){}

        address                                   play_owner;
        address                                   jackpot_owner;
        share_type                                play_amount;
        share_type                                jackpot_received;
        double                                  payouts;
        bool                                    roll_high;
        uint32_t                                  lucky_number;
        transaction_id_type                     dice_transaction_id;
    };
    typedef fc::optional<dice_record> odice_record;

} } // bts::blockchain

FC_REFLECT( bts::blockchain::dice_record,
           (id)(owner)(amount)(payouts)(roll_high)(jackpot_block_num)
           )
FC_REFLECT_TYPENAME( std::vector<bts::blockchain::jackpot_transaction> )
FC_REFLECT( bts::blockchain::jackpot_transaction,
           (play_owner)
           (jackpot_owner)
           (play_amount)
           (jackpot_received)
           (payouts)
            (roll_high)
           (lucky_number)
            (dice_transaction_id)
           )

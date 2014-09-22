#pragma once

#include <bts/blockchain/operations.hpp>
#include <bts/blockchain/types.hpp>
#include <bts/blockchain/withdraw_types.hpp>

namespace bts { namespace blockchain {

    struct dice_operation
    {
        static const operation_type_enum type;
        dice_operation():amount(0), payouts(2){}

        dice_operation( const address& owner, share_type amnt, double payouts = 2 );

        address owner()const;

        /** owner is just the hash of the condition */
        balance_id_type                balance_id()const;

        share_type          amount;
        double            payouts;

        /** the condition that the funds may be withdrawn,
         *  this is only necessary if the address is new.
         */
        withdraw_condition  condition;


        void evaluate( transaction_evaluation_state& eval_state );
    };

    /*
     * TODO: to replace with withdraw_pay_operation
     */
    /*
    struct jackpot_operation
    {
        static const operation_type_enum type;
        jackpot_operation():amount(0){}

        jackpot_operation( share_type amount_to_withdraw,
                               account_id_type id )
        :amount(amount_to_withdraw),account_id(id) {}

        share_type                       amount;
        account_id_type                  account_id;

        void evaluate( transaction_evaluation_state& eval_state );
    };
     */
} } // bts::blockchain

FC_REFLECT( bts::blockchain::dice_operation, (amount)(payouts)(condition) )
//FC_REFLECT( bts::blockchain::jackpot_operation, (amount)(account_id) )


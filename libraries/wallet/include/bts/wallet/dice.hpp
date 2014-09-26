#pragma once

#include <bts/blockchain/dice_record.hpp>
#include <bts/wallet/wallet_records.hpp>

namespace bts { namespace wallet {
   using namespace bts::blockchain;

    struct dice_transaction_record {
    	wallet_transaction_record transaction;
    	bool has_jackpot;
    	dice_record dice;
    	jackpot_transaction jackpot;
    };
}}
FC_REFLECT( bts::wallet::dice_transaction_record,
           (transaction)
           (has_jackpot)
           (dice)
           (jackpot)
           )

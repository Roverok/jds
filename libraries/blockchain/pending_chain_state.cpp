#include <bts/blockchain/pending_chain_state.hpp>
#include <bts/blockchain/dice_config.hpp>
#include <algorithm>

namespace bts { namespace blockchain {

   pending_chain_state::pending_chain_state( chain_interface_ptr prev_state )
   :_prev_state( prev_state )
   {
   }

   uint32_t pending_chain_state::get_head_block_num() const
   {
        auto state = _prev_state.lock();
        return state->get_head_block_num();
   }

   fc::ripemd160  pending_chain_state::get_current_random_seed()const
   {
      chain_interface_ptr prev_state = _prev_state.lock();
      if( prev_state )
         return prev_state->get_current_random_seed();
      return fc::ripemd160();
   }

   void pending_chain_state::set_prev_state( chain_interface_ptr prev_state )
   {
      _prev_state = prev_state;
   }

   pending_chain_state::~pending_chain_state(){}

   /**
    *  Based upon the current state of the database, calculate any updates that
    *  should be executed in a deterministic manner.
    */
   void pending_chain_state::apply_deterministic_updates()
   {
      /** nothing to do for now... charge 5% inactivity fee? */
      /** execute order matching */
   }

   /** polymorphically allcoate a new state */
   chain_interface_ptr pending_chain_state::create( const chain_interface_ptr& prev_state )const
   {
      return std::make_shared<pending_chain_state>(prev_state);
   }

   /** Apply changes from this pending state to the previous state */
   void pending_chain_state::apply_changes()const
   {
      chain_interface_ptr prev_state = _prev_state.lock();
      if( !prev_state ) return;
      for( const auto& item : properties )      prev_state->set_property( (chain_property_enum)item.first, item.second );
      for( const auto& item : assets )          prev_state->store_asset_record( item.second );
      for( const auto& item : accounts )        prev_state->store_account_record( item.second );
      for( const auto& item : balances )        prev_state->store_balance_record( item.second );
#if 0
      for( const auto& item : proposals )       prev_state->store_proposal_record( item.second );
      for( const auto& item : proposal_votes )  prev_state->store_proposal_vote( item.second );
#endif
      for( const auto& item : bids )            prev_state->store_bid_record( item.first, item.second );
      for( const auto& item : asks )            prev_state->store_ask_record( item.first, item.second );
      for( const auto& item : shorts )          prev_state->store_short_record( item.first, item.second );
      for( const auto& item : collateral )      prev_state->store_collateral_record( item.first, item.second );
      for( const auto& item : transactions )    prev_state->store_transaction( item.first, item.second );
      for( const auto& item : slates )          prev_state->store_delegate_slate( item.first, item.second );
      for( const auto& item : slots )           prev_state->store_slot_record( item.second );
      for( const auto& item : market_history )  prev_state->store_market_history_record( item.first, item.second );
      for( const auto& item : market_statuses ) prev_state->store_market_status( item.second );
      for( const auto& item : feeds )           prev_state->set_feed( item.second );
      for( const auto& items : recent_operations )
      {
         for( const auto& item : items.second )    prev_state->store_recent_operation( item );
      }
	    share_type accumulated_dice_amount = 0;
	       std::vector<dice_record> vals;
	       vals.reserve(dices.size());

	       for(auto kv : dices) {
	           vals.push_back(kv.second);
	       }

	       const auto sorter = []( const dice_record& a, const dice_record& b ) -> bool
	       {
	           return a.amount < b.amount;
	       };
	       std::sort( vals.begin(), vals.end(), sorter );

	  auto block_num = get_head_block_num()+1;
      for ( auto& dice : vals ) {
    	  if (dice.jackpot_block_num==0) {
    	    	odice_record dice_record = odice_record(dice);
    	    	auto amount = dice_record->amount * dice_record->payouts;
    	        accumulated_dice_amount+= amount;
    	    	int waitBlocks = BTS_BLOCKCHAIN_NUM_DICE;

    	auto pending_base_record = get_asset_record( asset_id_type( 0 ) );
    	FC_ASSERT( pending_base_record.valid() );
    	auto max_shares = pending_base_record->maximum_share_supply;
    	auto current_shares = pending_base_record->current_share_supply;
    	auto max_subsidy = (max_shares - current_shares) / P2P_DILUTION_RATE;
    	const auto max_available_paycheck = get_delegate_pay_rate();
    	        waitBlocks += (uint32_t)(accumulated_dice_amount / (max_subsidy+max_available_paycheck));
    	    	if (waitBlocks>=DICE_MAX_WAIT_BLOCKS)
    	    	    waitBlocks = DICE_MAX_WAIT_BLOCKS;
    	  dice.jackpot_block_num = block_num+waitBlocks;
    	  }
    	  prev_state->store_dice_record(dice);
      }
prev_state->set_jackpot_transactions( jackpot_transactions );
      prev_state->set_market_transactions( market_transactions );
      prev_state->set_dirty_markets(_dirty_markets);
   }

   otransaction_record pending_chain_state::get_transaction( const transaction_id_type& trx_id,
                                                              bool exact  )const
   {
      auto itr = transactions.find( trx_id );
      if( itr != transactions.end() ) return itr->second;
      chain_interface_ptr prev_state = _prev_state.lock();
      return prev_state->get_transaction( trx_id, exact );
   }

   bool pending_chain_state::is_known_transaction( const transaction_id_type& id )
   { try {
      auto itr = transactions.find( id );
      if( itr != transactions.end() ) return true;
      chain_interface_ptr prev_state = _prev_state.lock();
      return prev_state->is_known_transaction( id );
   } FC_CAPTURE_AND_RETHROW( (id) ) }

   void pending_chain_state::store_transaction( const transaction_id_type& id,
                                                const transaction_record& rec )
   {
      transactions[id] = rec;

      for( const auto& op : rec.trx.operations )
        store_recent_operation(op);
   }

   void pending_chain_state::get_undo_state( const chain_interface_ptr& undo_state_arg )const
   {
      auto undo_state = std::dynamic_pointer_cast<pending_chain_state>(undo_state_arg);
      chain_interface_ptr prev_state = _prev_state.lock();
      FC_ASSERT( prev_state );
      for( const auto& item : properties )
      {
         auto prev_value = prev_state->get_property( (chain_property_enum)item.first );
         undo_state->set_property( (chain_property_enum)item.first, prev_value );
      }
      for( const auto& item : assets )
      {
         auto prev_value = prev_state->get_asset_record( item.first );
         if( !!prev_value ) undo_state->store_asset_record( *prev_value );
         else undo_state->store_asset_record( item.second.make_null() );
      }
      for( const auto& item : slates )
      {
         auto prev_value = prev_state->get_delegate_slate( item.first );
         if( prev_value ) undo_state->store_delegate_slate( item.first, *prev_value );
         else undo_state->store_delegate_slate( item.first, delegate_slate() );
      }
      for( const auto& item : accounts )
      {
         auto prev_value = prev_state->get_account_record( item.first );
         if( !!prev_value ) undo_state->store_account_record( *prev_value );
         else undo_state->store_account_record( item.second.make_null() );
      }
#if 0
      for( const auto& item : proposals )
      {
         auto prev_value = prev_state->get_proposal_record( item.first );
         if( !!prev_value ) undo_state->store_proposal_record( *prev_value );
         else undo_state->store_proposal_record( item.second.make_null() );
      }
      for( const auto& item : proposal_votes )
      {
         auto prev_value = prev_state->get_proposal_vote( item.first );
         if( !!prev_value ) undo_state->store_proposal_vote( *prev_value );
         else { undo_state->store_proposal_vote( item.second.make_null() ); }
      }
#endif
      for( const auto& item : balances )
      {
         auto prev_value = prev_state->get_balance_record( item.first );
         if( !!prev_value ) undo_state->store_balance_record( *prev_value );
         else undo_state->store_balance_record( item.second.make_null() );
      }
      for( const auto& item : transactions )
      {
         auto prev_value = prev_state->get_transaction( item.first );
         if( !!prev_value ) undo_state->store_transaction( item.first, *prev_value );
         else undo_state->store_transaction( item.first, transaction_record() );
      }
      for( const auto& item : bids )
      {
         auto prev_value = prev_state->get_bid_record( item.first );
         if( prev_value.valid() ) undo_state->store_bid_record( item.first, *prev_value );
         else  undo_state->store_bid_record( item.first, order_record() );
      }
      for( const auto& item : asks )
      {
         auto prev_value = prev_state->get_ask_record( item.first );
         if( prev_value.valid() ) undo_state->store_ask_record( item.first, *prev_value );
         else  undo_state->store_ask_record( item.first, order_record() );
      }
      for( const auto& item : shorts )
      {
         auto prev_value = prev_state->get_short_record( item.first );
         if( prev_value.valid() ) undo_state->store_short_record( item.first, *prev_value );
         else  undo_state->store_short_record( item.first, order_record() );
      }
      for( const auto& item : collateral )
      {
         auto prev_value = prev_state->get_collateral_record( item.first );
         if( prev_value.valid() ) undo_state->store_collateral_record( item.first, *prev_value );
         else  undo_state->store_collateral_record( item.first, collateral_record() );
      }
      for( const auto& item : slots )
      {
         auto prev_value = prev_state->get_slot_record( item.first );
         if( prev_value ) undo_state->store_slot_record( *prev_value );
         else
         {
             slot_record invalid_slot_record;
             invalid_slot_record.start_time = item.first;
             invalid_slot_record.block_produced = true;
             invalid_slot_record.block_id = block_id_type();
             undo_state->store_slot_record( invalid_slot_record );
         }
      }
      for( const auto& item : market_statuses )
      {
         auto prev_value = prev_state->get_market_status( item.first.first, item.first.second );
         if( prev_value ) undo_state->store_market_status( *prev_value );
         else
         {
            undo_state->store_market_status( market_status() );
         }
      }
      for( auto const& item : feeds )
      {
         auto prev_value = prev_state->get_feed( item.first );
         if( prev_value ) undo_state->set_feed( *prev_value );
         else undo_state->set_feed( feed_record{item.first} );
      }
      for ( const auto& item : dices ) {
          auto prev_value = prev_state->get_dice_record(item.first);
          if( !! prev_state ) undo_state->store_dice_record( *prev_value );
          else undo_state->store_dice_record( item.second.make_null() );
      }

      auto dirty_markets = prev_state->get_dirty_markets();
      undo_state->set_dirty_markets(dirty_markets);

      /* NOTE: Recent operations are currently not rewound on undo */
   }

   /** load the state from a variant */
   void pending_chain_state::from_variant( const fc::variant& v )
   {
      fc::from_variant( v, *this );
   }

   /** convert the state to a variant */
   fc::variant pending_chain_state::to_variant()const
   {
      fc::variant v;
      fc::to_variant( *this, v );
      return v;
   }

   oasset_record pending_chain_state::get_asset_record( const asset_id_type& asset_id )const
   {
      chain_interface_ptr prev_state = _prev_state.lock();
      auto itr = assets.find( asset_id );
      if( itr != assets.end() )
        return itr->second;
      else if( prev_state )
        return prev_state->get_asset_record( asset_id );
      return oasset_record();
   }

   oasset_record pending_chain_state::get_asset_record( const std::string& symbol )const
   {
      chain_interface_ptr prev_state = _prev_state.lock();
      auto itr = symbol_id_index.find( symbol );
      if( itr != symbol_id_index.end() )
        return get_asset_record( itr->second );
      else if( prev_state )
        return prev_state->get_asset_record( symbol );
      return oasset_record();
   }

   fc::time_point_sec pending_chain_state::now()const
   {
      chain_interface_ptr prev_state = _prev_state.lock();
      if( prev_state )
        return prev_state->now();
      FC_ASSERT( false, "No current timestamp set" );
   }

   obalance_record pending_chain_state::get_balance_record( const balance_id_type& balance_id )const
   {
      chain_interface_ptr prev_state = _prev_state.lock();
      auto itr = balances.find( balance_id );
      if( itr != balances.end() )
        return itr->second;
      else if( prev_state )
        return prev_state->get_balance_record( balance_id );
      return obalance_record();
   }

   odelegate_slate pending_chain_state::get_delegate_slate( slate_id_type id )const
   {
      chain_interface_ptr prev_state = _prev_state.lock();
      auto itr = slates.find(id);
      if( itr != slates.end() ) return itr->second;
      if( prev_state ) return prev_state->get_delegate_slate( id );
      return odelegate_slate();
   }

   void pending_chain_state::store_delegate_slate( slate_id_type id, const delegate_slate& slate )
   {
      slates[id] = slate;
   }

   oaccount_record pending_chain_state::get_account_record( const address& owner )const
   {
      auto itr = key_to_account.find(owner);
      if( itr != key_to_account.end() ) return get_account_record( itr->second );
      chain_interface_ptr prev_state = _prev_state.lock();
      FC_ASSERT(prev_state);
      return prev_state->get_account_record( owner );
   }

   oaccount_record pending_chain_state::get_account_record( const account_id_type& account_id )const
   {
      chain_interface_ptr prev_state = _prev_state.lock();
      auto itr = accounts.find( account_id );
      if( itr != accounts.end() )
        return itr->second;
      else if( prev_state )
        return prev_state->get_account_record( account_id );
      return oaccount_record();
   }

   oaccount_record pending_chain_state::get_account_record( const std::string& name )const
   {
      chain_interface_ptr prev_state = _prev_state.lock();
      auto itr = account_id_index.find( name );
      if( itr != account_id_index.end() )
        return get_account_record( itr->second );
      else if( prev_state )
        return prev_state->get_account_record( name );
      return oaccount_record();
   }

   void pending_chain_state::store_asset_record( const asset_record& r )
   {
      assets[r.id] = r;
   }

   void pending_chain_state::store_balance_record( const balance_record& r )
   {
      balances[r.id()] = r;
   }

   void pending_chain_state::store_account_record( const account_record& r )
   {
      accounts[r.id] = r;
      account_id_index[r.name] = r.id;
      for( const auto& item : r.active_key_history )
         key_to_account[address(item.second)] = r.id;
      key_to_account[address(r.owner_key)] = r.id;
   }

   vector<operation> pending_chain_state::get_recent_operations(operation_type_enum t)
   {
      const auto& recent_op_queue = recent_operations[t];
      vector<operation> recent_ops(recent_op_queue.size());
      std::copy(recent_op_queue.begin(), recent_op_queue.end(), recent_ops.begin());
      return recent_ops;
   }

   void pending_chain_state::store_recent_operation(const operation& o)
   {
      auto& recent_op_queue = recent_operations[o.type];
      recent_op_queue.push_back(o);
      if( recent_op_queue.size() > MAX_RECENT_OPERATIONS )
        recent_op_queue.pop_front();
   }

   fc::variant pending_chain_state::get_property( chain_property_enum property_id )const
   {
      auto property_itr = properties.find( property_id );
      if( property_itr != properties.end()  ) return property_itr->second;
      chain_interface_ptr prev_state = _prev_state.lock();
      if( prev_state ) return prev_state->get_property( property_id );
      return fc::variant();
   }

   void pending_chain_state::set_property( chain_property_enum property_id,
                                                     const fc::variant& property_value )
   {
      properties[property_id] = property_value;
   }

#if 0
   void pending_chain_state::store_proposal_record( const proposal_record& r )
   {
      proposals[r.id] = r;
   }

   oproposal_record pending_chain_state::get_proposal_record( proposal_id_type id )const
   {
      chain_interface_ptr prev_state = _prev_state.lock();
      auto rec_itr = proposals.find(id);
      if( rec_itr != proposals.end() ) return rec_itr->second;
      else if( prev_state ) return prev_state->get_proposal_record( id );
      return oproposal_record();
   }

   void pending_chain_state::store_proposal_vote( const proposal_vote& r )
   {
      proposal_votes[r.id] = r;
   }

   oproposal_vote pending_chain_state::get_proposal_vote( proposal_vote_id_type id )const
   {
      chain_interface_ptr prev_state = _prev_state.lock();
      auto rec_itr = proposal_votes.find(id);
      if( rec_itr != proposal_votes.end() ) return rec_itr->second;
      else if( prev_state ) return prev_state->get_proposal_vote( id );
      return oproposal_vote();
   }
#endif

   oorder_record pending_chain_state::get_bid_record( const market_index_key& key )const
   {
      chain_interface_ptr prev_state = _prev_state.lock();
      auto rec_itr = bids.find( key );
      if( rec_itr != bids.end() ) return rec_itr->second;
      else if( prev_state ) return prev_state->get_bid_record( key );
      return oorder_record();
   }

   omarket_order pending_chain_state::get_lowest_ask_record( const asset_id_type& quote_id, const asset_id_type& base_id )
   {
      chain_interface_ptr prev_state = _prev_state.lock();
      omarket_order result;
      if( prev_state )
      {
        auto pending = prev_state->get_lowest_ask_record( quote_id, base_id );
        if( pending )
        {
           pending->state = *get_ask_record( pending->market_index );
        }
        return pending;
      }
      return result;
   }

   oorder_record pending_chain_state::get_ask_record( const market_index_key& key )const
   {
      chain_interface_ptr prev_state = _prev_state.lock();
      auto rec_itr = asks.find( key );
      if( rec_itr != asks.end() ) return rec_itr->second;
      else if( prev_state ) return prev_state->get_ask_record( key );
      return oorder_record();
   }

   oorder_record pending_chain_state::get_short_record( const market_index_key& key )const
   {
      chain_interface_ptr prev_state = _prev_state.lock();
      auto rec_itr = shorts.find( key );
      if( rec_itr != shorts.end() ) return rec_itr->second;
      else if( prev_state ) return prev_state->get_short_record( key );
      return oorder_record();
   }

   ocollateral_record pending_chain_state::get_collateral_record( const market_index_key& key )const
   {
      chain_interface_ptr prev_state = _prev_state.lock();
      auto rec_itr = collateral.find( key );
      if( rec_itr != collateral.end() ) return rec_itr->second;
      else if( prev_state ) return prev_state->get_collateral_record( key );
      return ocollateral_record();
   }

   void pending_chain_state::store_bid_record( const market_index_key& key, const order_record& rec )
   {
      bids[key] = rec;
      _dirty_markets[key.order_price.quote_asset_id] = key.order_price.base_asset_id;
   }

   void pending_chain_state::store_ask_record( const market_index_key& key, const order_record& rec )
   {
      asks[key] = rec;
      _dirty_markets[key.order_price.quote_asset_id] = key.order_price.base_asset_id;
   }

   void pending_chain_state::store_short_record( const market_index_key& key, const order_record& rec )
   {
      shorts[key] = rec;
      _dirty_markets[key.order_price.quote_asset_id] = key.order_price.base_asset_id;
   }

   void pending_chain_state::set_market_dirty( const asset_id_type& quote_id, const asset_id_type& base_id )
   {
      _dirty_markets[quote_id] = base_id;
   }

   void pending_chain_state::store_collateral_record( const market_index_key& key, const collateral_record& rec )
   {
      collateral[key] = rec;
      _dirty_markets[key.order_price.quote_asset_id] = key.order_price.base_asset_id;
   }

   void pending_chain_state::store_slot_record( const slot_record& r )
   {
      slots[ r.start_time ] = r;
   }

   oslot_record pending_chain_state::get_slot_record( const time_point_sec& start_time )const
   {
      chain_interface_ptr prev_state = _prev_state.lock();
      auto itr = slots.find( start_time );
      if( itr != slots.end() ) return itr->second;
      if( prev_state ) return prev_state->get_slot_record( start_time );
      return oslot_record();
   }

   void pending_chain_state::store_market_history_record(const market_history_key& key, const market_history_record& record)
   {
     market_history[key] = record;
   }

   omarket_history_record pending_chain_state::get_market_history_record(const market_history_key& key) const
   {
     if( market_history.find(key) != market_history.end() )
       return market_history.find(key)->second;
     return omarket_history_record();
   }

   void pending_chain_state::set_market_transactions( vector<market_transaction> trxs )
   {
      market_transactions = std::move(trxs);
   }
void pending_chain_state::set_jackpot_transactions( vector<jackpot_transaction> trxs )
{
   jackpot_transactions = std::move(trxs);
}

   omarket_status pending_chain_state::get_market_status( const asset_id_type& quote_id, const asset_id_type& base_id )
   {
      auto itr = market_statuses.find( std::make_pair(quote_id,base_id) );
      if( itr != market_statuses.end() )
         return itr->second;
      chain_interface_ptr prev_state = _prev_state.lock();
      return prev_state->get_market_status(quote_id,base_id);
   }

   void pending_chain_state::store_market_status( const market_status& s )
   {
      market_statuses[std::make_pair(s.quote_id,s.base_id)] = s;
   }

   void pending_chain_state::set_feed( const feed_record& r )
   {
      feeds[r.feed] = r;
   }

   ofeed_record    pending_chain_state::get_feed( const feed_index& i )const
   {
      auto itr = feeds.find(i);
      if( itr != feeds.end() ) return itr->second;

      chain_interface_ptr prev_state = _prev_state.lock();
      return prev_state->get_feed(i);
   }

   oprice pending_chain_state::get_median_delegate_price( const asset_id_type& asset_id )const
   {
      chain_interface_ptr prev_state = _prev_state.lock();
      return prev_state->get_median_delegate_price( asset_id );
   }

   asset pending_chain_state::calculate_base_supply()const
   {
      auto total = asset( 0, 0 );

      for( const auto& balance_item : balances )
      {
        const balance_record balance = balance_item.second;
        if( balance.asset_id() != total.asset_id ) continue;
        total += balance.get_balance();
      }

      total.amount += get_accumulated_fees();

      for( const auto& account_item : accounts )
      {
        const account_record account = account_item.second;
        if( !account.delegate_info.valid() ) continue;
        total.amount += account.delegate_info->pay_balance;
      }

      for( const auto& ask_item : asks )
      {
        const order_record ask = ask_item.second;
        total.amount += ask.balance;
      }

      for( const auto& short_item : shorts )
      {
        const order_record sh = short_item.second;
        total.amount += sh.balance;
      }

      for( const auto& collateral_item : collateral )
      {
        const collateral_record col = collateral_item.second;
        total.amount += col.collateral_balance;
      }

      return total;
   }
   odice_record pending_chain_state::get_dice_record( const dice_id_type& dice_id )const
   {
       chain_interface_ptr prev_state = _prev_state.lock();
       auto itr = dices.find( dice_id );
       if( itr != dices.end() )
           return itr->second;
       else if( prev_state )
           return prev_state->get_dice_record( dice_id );
       return odice_record();
   }
   void pending_chain_state::store_dice_record( const dice_record& r )
   {
	  dices[r.id] = r;
   }

} } // bts::blockchain

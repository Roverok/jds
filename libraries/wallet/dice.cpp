#include <bts/wallet/wallet.hpp>
#include <bts/wallet/dice.hpp>

namespace bts { namespace client {
namespace detail { class client_impl; }
using std::vector;
using std::string;

using bts::wallet::dice_transaction_record;
//vector<dice_transaction_record> detail::client_impl::wallet_account_dice_transaction_history( const string& account_name,
//                                                                                    const string& asset_symbol,
//                                                                                    int32_t limit,
//                                                                                    uint32_t start_block_num,
//                                                                                    uint32_t end_block_num )const
//{
//	try{
//	      vector<dice_transaction_record> history;
//	      vector<wallet_transaction_record> tx_history = _wallet->get_transaction_history(account_name, start_block_num, end_block_num, asset_symbol);
//	       for( const auto& item : tx_history ) {
//	    	   dice_transaction_record record;
//	    	   record.transaction = item;
//	    	   history.push_back( record );
//	       }
//
////    	      const auto history = _wallet->get_pretty_transaction_history( account_name, start_block_num, end_block_num, asset_symbol );
//	      if( limit == 0 || abs( limit ) >= history.size() )
//	      {
//	          return history;
//	      }
//	      else if( limit > 0 )
//	      {
//	          return vector<dice_transaction_record>( history.begin(), history.begin() + limit );
//	      }
//	      else
//	      {
//	          return vector<dice_transaction_record>( history.end() - abs( limit ), history.end() );
//	      }
//	      return history_records;
//} FC_RETHROW_EXCEPTIONS( warn, "") }

}}

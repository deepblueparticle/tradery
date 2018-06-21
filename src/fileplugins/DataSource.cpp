/*
Copyright (C) 2018 Adrian Michel
http://www.amichel.com

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "stdafx.h"
#include "datasource.h"

/**
 * Parse tick line
 * tick line format:
 * data/time price shares type exchange
 * where type can be:
 * - bid
 * - ask
 * - best bid
 * - best ask
 * - trade
 *
 * Each line contains a bar
 *
 * @param str
 * @param lineCount
 * @return
 * @exception DataSourceException
 */
/*
const tradery::Tick* FileDataSource::parseTickLine( const std::wstring& str )
const throw ( DataSourceException )
{
  if ( str.empty() )
    return 0;
  // comment line starts with // or #
  if ( str.at( 0 ) == _TCHAR('#') || str.length() > 1 && str.at( 0 ) ==
_TCHAR('/') && str.at( 1 ) == _TCHAR('/') ) return 0;

  //TODO - check data format
  vector< std::wstring > tokens;

  CIsFromString< TCHAR > sep( _T( ", \t" ) );
  CTokenizer< TCHAR, CIsFromString< TCHAR > >::tokenize( tokens, str, sep );

  int n = 0;
  std::vector< std::wstring > vs;

  std::wostringstream o;
  for ( vector< std::wstring >::iterator tok_iter = tokens.begin(); tok_iter !=
tokens.end(); ++tok_iter, n++ )
  {
    vs.push_back( *tok_iter );
    std::wstring s( *tok_iter );
//      o << s << ", ";
  }
  o << std::endl;

  // TODO: check that is valid
  // TODO: prepare for futures (more fields - open interest)

  std::wstring type = vs[ 4 ];
  std::wstring exchange;

  TickType t;

  to_lower_case( type );

  if( type == _T( "ask" ) )
  {
    t = ASK;
    exchange = vs[ 5 ];
  }
  else if( type == _T( "bid" ) )
  {
    t = BID;
    exchange = vs[ 5 ];
  }
  else if( type == _T( "trade" ) )
  {
    t = TRADE;
    exchange = vs[ 5 ];
  }
  else if( type == _T( "best" ) )
  {
    std::wstring x( vs[ 5 ] );
    to_lower_case( x );
    if( x == _T( "ask" ) )
      t = BEST_ASK;
    else if( x == _T( "bid" ) )
      t = BEST_BID;
    else
    {
      // not a valid type
      //error
    }
    exchange = vs[ 6 ];
  }

  TCHAR* p;
  return new Tick( parseDate( vs[ 0 ], vs[ 1 ] ), _tcstod( vs[ 2 ].c_str(), &p
), _ttol( vs[ 3 ].c_str() ), t, exchange );
}
*/

/*
void FileDataSource::parseTicks( tradery::Addable< Tick >* ticks, t_ifstream&
_file, const Range* range ) const throw ( BarException )
{
  std::wstring str;

  if ( range )
  {
    do
    {
      std::getline( _file, str );
      tradery::TickPtr pTick;

      if ( ( pTick = tradery::TickPtr( parseTickLine( str ) ) ).get() )
      {
        if ( *range > *pTick )
          continue;
        else if ( *range < *pTick )
          break;
        else
          ticks -> add( *pTick );

      }
    }
    while ( !_file.eof() );
  }
  else
  {
    do
    {
      std::getline( _file, str );
      TickPtr pTick;
      size_t count = 0;

      if ( ( pTick = TickPtr( parseTickLine( str ) ) ).get() )
        ticks ->add( *pTick );
    }
    while ( !_file.eof() );
  }
}
*/

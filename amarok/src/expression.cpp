/*
  Copyright (c) 2006 Gábor Lehel <illissius@gmail.com>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/

#include "expression.h"

ExpressionParser::ExpressionParser( const QString &expression )
    : m_expression( expression )
    , m_state( ExpectMinus )
    , m_haveGroup( false )
    , m_inQuote( false )
    , m_inOrGroup( false )
{ }

ParsedExpression ExpressionParser::parse()
{
    const uint length = m_expression.length();
    for( uint pos = 0; pos < length; ++pos )
        parseChar( m_expression.constref( pos ) );
    finishedToken();
    finishedOrGroup();
    return m_parsed;
}

ParsedExpression ExpressionParser::parse( const QString &expression ) //static
{
    ExpressionParser p( expression );
    return p.parse();
}

bool ExpressionParser::isAdvancedExpression( const QString &expression ) //static
{
    return ( expression.contains( '"'   ) ||
             expression.contains( ':'   ) ||
             expression.contains( '-'   ) ||
             expression.contains( "AND" ) ||
             expression.contains( "OR"  ) );
}

/* PRIVATE */

void ExpressionParser::parseChar( const QChar &c )
{
    if( m_inQuote && c != '"' )
        m_string += c;
    else if( c.isSpace() )
        handleSpace( c );
    else if( c == '-' )
        handleMinus( c );
    else if( c == ':' )
        handleColon( c );
    else if( c == '>' || c == '<' )
        handleMod( c );
    else if( c == '"' )
        handleQuote( c );
    else
        handleChar( c );
}

void ExpressionParser::handleSpace( const QChar& )
{
    if( m_state > ExpectMinus )
        finishedToken();
}

void ExpressionParser::handleMinus( const QChar &c )
{
    if( m_state == ExpectMinus )
    {
        m_element.negate = true;
        m_state = ExpectField;
    }
    else
        handleChar( c );
}

void ExpressionParser::handleColon( const QChar &c )
{
    if( m_state <= ExpectField && !m_string.isEmpty() )
    {
        m_element.field = m_string;
        m_string = QString::null;
        m_state = ExpectMod;
    }
    else
        handleChar( c );
}

void ExpressionParser::handleMod( const QChar &c )
{
    if( m_state == ExpectMod )
    {
        m_element.match = ( c == '>' ) ? expression_element::More : expression_element::Less;
        m_state = ExpectText;
    }
    else
        handleChar( c );
}

void ExpressionParser::handleQuote( const QChar& )
{
    if( m_inQuote )
    {
        finishedElement();
        m_inQuote = false;
    }
    else
    {
        if( !m_string.isEmpty() )
            finishedToken();
        m_state = ExpectText;
        m_inQuote = true;
    }
}

void ExpressionParser::handleChar( const QChar &c )
{
    m_string += c;
    if( m_state <= ExpectField )
        m_state = ExpectField;
    else if( m_state <= ExpectText )
        m_state = ExpectText;
}

void ExpressionParser::finishedToken()
{
    enum { And, Or, Neither };
    int s;
    if( m_haveGroup || !m_element.field.isEmpty() )
        s = Neither;
    else if( m_string == "AND" )
        s = And;
    else if( m_string == "OR" )
        s = Or;
    else
        s = Neither;

    if( s == Neither )
        finishedElement();
    else
    {
        m_haveGroup = true;

        if( s == Or )
            m_inOrGroup = true;
        else
            finishedOrGroup();

        m_string = QString::null;
        m_state = ExpectMinus;
    }
}

void ExpressionParser::finishedElement()
{
    if( !m_inOrGroup )
        finishedOrGroup();
    m_inOrGroup = m_haveGroup = false;
    m_element.text = m_string;
    m_string = QString::null;

    if( !m_element.text.isEmpty() || !m_element.field.isEmpty() )
        m_or.append( m_element );

    m_element = expression_element();
    m_state = ExpectMinus;
}

void ExpressionParser::finishedOrGroup()
{
    if( !m_or.isEmpty() )
        m_parsed.append( m_or );
    m_or.clear();
    m_inOrGroup = false;
}


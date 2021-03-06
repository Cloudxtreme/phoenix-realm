/****************************************************************************
 *                                                                          *
 *   X      X  ******* **    **  ******  **    **  ******                   *
 *    X    X  ******** ***  *** ******** **    ** ********       \\._.//    *
 *     X  X   **       ******** **    ** **    ** **             (0...0)    *
 *      XX    *******  ******** ******** **    ** **  ****        ).:.(     *
 *      XX     ******* ** ** ** ******** **    ** **  ****        {o o}     *
 *     X  X         ** **    ** **    ** **    ** **    **       / ' ' \    *
 *    X    X  ******** **    ** **    ** ******** ********    -^^.VxvxV.^^- *
 *   X      X *******  **    ** **    **  ******   ******                   *
 *                                                                          *
 * ------------------------------------------------------------------------ *
 * Ne[X]t Generation [S]imulated [M]edieval [A]dventure Multi[U]ser [G]ame  *
 * ------------------------------------------------------------------------ *
 * XSMAUG 2.4 (C) 2014  by Antonio Cao @burzumishi          |    \\._.//    *
 * ---------------------------------------------------------|    (0...0)    *
 * SMAUG 1.4 (C) 1994, 1995, 1996, 1998  by Derek Snider    |     ).:.(     *
 * SMAUG Code Team: Thoric, Altrag, Blodkai, Narn, Haus,    |     {o o}     *
 * Scryn, Rennard, Swordbearer, Gorog, Grishnakh, Nivek,    |    / ' ' \    *
 * Tricops and Fireblade                                    | -^^.VxvxV.^^- *
 * ------------------------------------------------------------------------ *
 * Merc 2.1 Diku Mud improvments copyright (C) 1992, 1993 by Michael        *
 * Chastain, Michael Quan, and Mitchell Tse.                                *
 * Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,          *
 * Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.     *
 * Win32 port by Nick Gammon                                                *
 * ------------------------------------------------------------------------ *
 * AFKMud Copyright 1997-2012 by Roger Libiez (Samson),                     *
 * Levi Beckerson (Whir), Michael Ward (Tarl), Erik Wolfe (Dwip),           *
 * Cameron Carroll (Cam), Cyberfox, Karangi, Rathian, Raine,                *
 * Xorith, and Adjani.                                                      *
 * All Rights Reserved.                                                     *
 *                                                                          *
 * External contributions from Remcon, Quixadhal, Zarius, and many others.  *
 *                                                                          *
 ****************************************************************************
 *                         Table load/save Module                           *
 ****************************************************************************/

#if !defined(WIN32)
#include <dlfcn.h>
#else
#include <windows.h>
#define dlsym( handle, name ) ( (void*)GetProcAddress( (HINSTANCE) (handle), (name) ) )
#define dlerror() GetLastError()
#endif
#include "mud.h"
#include "language.h"

SPELLF( spell_notfound );

list < lang_data * >langlist;

SPELL_FUN *spell_function( const string & name )
{
   void *funHandle;
#if !defined(WIN32)
   const char *error;
#else
   DWORD error;
#endif

   funHandle = dlsym( sysdata->dlHandle, name.c_str(  ) );
   if( ( error = dlerror(  ) ) )
      return ( SPELL_FUN * ) spell_notfound;

   return ( SPELL_FUN * ) funHandle;
}

DO_FUN *skill_function( const string & name )
{
   void *funHandle;
#if !defined(WIN32)
   const char *error;
#else
   DWORD error;
#endif

   funHandle = dlsym( sysdata->dlHandle, name.c_str(  ) );
   if( ( error = dlerror(  ) ) )
      return ( DO_FUN * ) skill_notfound;

   return ( DO_FUN * ) funHandle;
}

lcnv_data::lcnv_data(  )
{
   old.clear(  );
   lnew.clear(  );
}

lang_data::lang_data(  )
{
}

lang_data::~lang_data(  )
{
   list < lcnv_data * >::iterator lcnv;

   for( lcnv = prelist.begin(  ); lcnv != prelist.end(  ); )
   {
      lcnv_data *lpre = *lcnv;
      ++lcnv;

      prelist.remove( lpre );
      deleteptr( lpre );
   }

   for( lcnv = cnvlist.begin(  ); lcnv != cnvlist.end(  ); )
   {
      lcnv_data *lc = *lcnv;
      ++lcnv;

      cnvlist.remove( lc );
      deleteptr( lc );
   }
   langlist.remove( this );
}

void free_tongues( void )
{
   list < lang_data * >::iterator ln;

   for( ln = langlist.begin(  ); ln != langlist.end(  ); )
   {
      lang_data *lang = *ln;
      ++ln;

      deleteptr( lang );
   }
}

/*
 * Tongues / Languages loading/saving functions - Altrag
 */
void fread_cnv( FILE * fp, lang_data * lng, bool pre )
{
   lcnv_data *cnv;
   char letter;

   for( ;; )
   {
      letter = fread_letter( fp );
      if( letter == '~' || letter == EOF )
         break;
      ungetc( letter, fp );

      cnv = new lcnv_data;
      cnv->old = fread_word( fp );
      cnv->lnew = fread_word( fp );
      fread_to_eol( fp );
      if( pre )
         lng->prelist.push_back( cnv );
      else
         lng->cnvlist.push_back( cnv );
   }
}

void load_tongues(  )
{
   FILE *fp;
   lang_data *lng;
   char *word;
   char letter;

   if( !( fp = fopen( TONGUE_FILE, "r" ) ) )
   {
      perror( "Load_tongues" );
      return;
   }
   for( ;; )
   {
      letter = fread_letter( fp );
      if( letter == EOF )
         break;
      else if( letter == '*' )
      {
         fread_to_eol( fp );
         continue;
      }
      else if( letter != '#' )
      {
         bug( "%s: Letter '%c' not #.", __FUNCTION__, letter );
         exit( 1 );
      }
      word = fread_word( fp );
      if( !str_cmp( word, "end" ) )
         break;
      fread_to_eol( fp );
      lng = new lang_data;
      lng->name = word;
      fread_cnv( fp, lng, true );
      fread_string( lng->alphabet, fp );
      fread_cnv( fp, lng, false );
      fread_to_eol( fp );
      langlist.push_back( lng );
   }
   FCLOSE( fp );
}

#include <wchar.h>
#include <stdlib.h>
#include <string.h>
#ifdef WIN32
#include <windows.h>
#endif
#include "unicode.h"
#include "exceptions.h"


/// Returns length of wide character in utf-8
#define UTF8_LENGTH(Char)              \
  ((Char) < 0x80 ? 1 :                 \
   ((Char) < 0x800 ? 2 :               \
    ((Char) < 0x10000 ? 3 :            \
     ((Char) < 0x200000 ? 4 :          \
      ((Char) < 0x4000000 ? 5 : 6)))))

#define UTF8_COMPUTE(Char, Mask, Len)					      \
  if (Char < 128)							      \
    {									      \
      Len = 1;								      \
      Mask = 0x7f;							      \
    }									      \
  else if ((Char & 0xe0) == 0xc0)					      \
    {									      \
      Len = 2;								      \
      Mask = 0x1f;							      \
    }									      \
  else if ((Char & 0xf0) == 0xe0)					      \
    {									      \
      Len = 3;								      \
      Mask = 0x0f;							      \
    }									      \
  else if ((Char & 0xf8) == 0xf0)					      \
    {									      \
      Len = 4;								      \
      Mask = 0x07;							      \
    }									      \
  else if ((Char & 0xfc) == 0xf8)					      \
    {									      \
      Len = 5;								      \
      Mask = 0x03;							      \
    }									      \
  else if ((Char & 0xfe) == 0xfc)					      \
    {									      \
      Len = 6;								      \
      Mask = 0x01;							      \
    }									      \
  else									      \
    Len = -1;



#ifndef WIN32

static const char utf8_skip_data[256] = {
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
  3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,4,4,4,4,4,4,4,4,5,5,5,5,6,6,1,1
};

const char * const g_utf8_skip = utf8_skip_data;

#define g_utf8_next_char(p) (char *)((p) + g_utf8_skip[*(unsigned char *)(p)])

#define UTF8_GET(Result, Chars, Count, Mask, Len)			      \
  (Result) = (Chars)[0] & (Mask);					      \
  for ((Count) = 1; (Count) < (Len); ++(Count))				      \
    {									      \
      if (((Chars)[(Count)] & 0xc0) != 0x80)				      \
	{								      \
	  (Result) = -1;						      \
	  break;							      \
	}								      \
      (Result) <<= 6;							      \
      (Result) |= ((Chars)[(Count)] & 0x3f);				      \
    }

/* Like g_utf8_get_char, but take a maximum length
 * and return (wchar_t)-2 on incomplete trailing character
 */
static inline wchar_t
g_utf8_get_char_extended (const  char *p,
			  size_t max_len)  
{
  unsigned int i, len;
  wchar_t wc = (unsigned char) *p;

  if (wc < 0x80)
    {
      return wc;
    }
  else if (wc < 0xc0)
    {
      return (wchar_t)-1;
    }
  else if (wc < 0xe0)
    {
      len = 2;
      wc &= 0x1f;
    }
  else if (wc < 0xf0)
    {
      len = 3;
      wc &= 0x0f;
    }
  else if (wc < 0xf8)
    {
      len = 4;
      wc &= 0x07;
    }
  else if (wc < 0xfc)
    {
      len = 5;
      wc &= 0x03;
    }
  else if (wc < 0xfe)
    {
      len = 6;
      wc &= 0x01;
    }
  else
    {
      return (wchar_t)-1;
    }
  
  if (max_len >= 0 && len > max_len)
    {
      for (i = 1; i < max_len; i++)
	{
	  if ((((unsigned char *)p)[i] & 0xc0) != 0x80)
	    return (wchar_t)-1;
	}
      return (wchar_t)-2;
    }

  for (i = 1; i < len; ++i)
    {
      wchar_t ch = ((unsigned char *)p)[i];
      
      if ((ch & 0xc0) != 0x80)
	{
	  if (ch)
	    return (wchar_t)-1;
	  else
	    return (wchar_t)-2;
	}

      wc <<= 6;
      wc |= (ch & 0x3f);
    }

  if (UTF8_LENGTH(wc) != len)
    return (wchar_t)-1;
  
  return wc;
}

/**
 * g_utf8_get_char:
 * @p: a pointer to Unicode character encoded as UTF-8
 * 
 * Converts a sequence of bytes encoded as UTF-8 to a Unicode character.
 * If @p does not point to a valid UTF-8 encoded character, results are
 * undefined. If you are not sure that the bytes are complete
 * valid Unicode characters, you should use g_utf8_get_char_validated()
 * instead.
 * 
 * Return value: the resulting character
 **/
wchar_t
g_utf8_get_char (const char *p)
{
  int i, mask = 0, len;
  wchar_t result;
  unsigned char c = (unsigned char) *p;

  UTF8_COMPUTE (c, mask, len);
  if (len == -1)
    return (wchar_t)-1;
  UTF8_GET (result, p, i, mask, len);

  return result;
}


/**
 * g_utf8_to_ucs4:
 * @str: a UTF-8 encoded string
 * @len: the maximum length of @str to use. If @len < 0, then
 *       the string is nul-terminated.
 * @items_read: location to store number of bytes read, or %NULL.
 *              If %NULL, then %G_CONVERT_ERROR_PARTIAL_INPUT will be
 *              returned in case @str contains a trailing partial
 *              character. If an error occurs then the index of the
 *              invalid input is stored here.
 * @items_written: location to store number of characters written or %NULL.
 *                 The value here stored does not include the trailing 0
 *                 character. 
 * @error: location to store the error occuring, or %NULL to ignore
 *         errors. Any of the errors in #GConvertError other than
 *         %G_CONVERT_ERROR_NO_CONVERSION may occur.
 *
 * Convert a string from UTF-8 to a 32-bit fixed width
 * representation as UCS-4. A trailing 0 will be added to the
 * string after the converted text.
 * 
 * Return value: a pointer to a newly allocated UCS-4 string.
 *               This value must be freed with g_free(). If an
 *               error occurs, %NULL will be returned and
 *               @error set.
 **/
wchar_t *
g_utf8_to_ucs4 (const char *str,
		long        len,             
		long       *items_read,      
		long       *items_written,   
		wchar_t **error)
{
  wchar_t *result = NULL;
  int n_chars, i;
  const char *in;
  
  in = str;
  n_chars = 0;
  while ((len < 0 || str + len - in > 0) && *in)
    {
      wchar_t wc = g_utf8_get_char_extended (in, str + len - in);
      if (wc & 0x80000000)
	{
	  if (wc == (wchar_t)-2)
	    {
	      if (items_read)
		break;
	      else
                if (error)
		  *error = L"Partial character sequence at end of input";
	    }
	  else
            if (error)
              *error = L"Invalid byte sequence in conversion input";

	  goto err_out;
	}

      n_chars++;

      in = g_utf8_next_char (in);
    }

  result = (wchar_t*)malloc((n_chars + 1) * sizeof(wchar_t));
  
  in = str;
  for (i=0; i < n_chars; i++)
    {
      result[i] = g_utf8_get_char (in);
      in = g_utf8_next_char (in);
    }
  result[i] = 0;

  if (items_written)
    *items_written = n_chars;

 err_out:
  if (items_read)
    *items_read = in - str;

  return result;
}

/**
 * g_unichar_to_utf8:
 * @c: a ISO10646 character code
 * @outbuf: output buffer, must have at least 6 bytes of space.
 *       If %NULL, the length will be computed and returned
 *       and nothing will be written to @outbuf.
 * 
 * Converts a single character to UTF-8.
 * 
 * Return value: number of bytes written
 **/
int
g_unichar_to_utf8 (wchar_t c,
		   char   *outbuf)
{
  unsigned int len = 0;    
  int first;
  int i;

  if (c < 0x80)
    {
      first = 0;
      len = 1;
    }
  else if (c < 0x800)
    {
      first = 0xc0;
      len = 2;
    }
  else if (c < 0x10000)
    {
      first = 0xe0;
      len = 3;
    }
   else if (c < 0x200000)
    {
      first = 0xf0;
      len = 4;
    }
  else if (c < 0x4000000)
    {
      first = 0xf8;
      len = 5;
    }
  else
    {
      first = 0xfc;
      len = 6;
    }

  if (outbuf)
    {
      for (i = len - 1; i > 0; --i)
	{
	  outbuf[i] = (c & 0x3f) | 0x80;
	  c >>= 6;
	}
      outbuf[0] = c | first;
    }

  return len;
}

/**
 * g_ucs4_to_utf8:
 * @str: a UCS-4 encoded string
 * @len: the maximum length of @str to use. If @len < 0, then
 *       the string is terminated with a 0 character.
 * @items_read: location to store number of characters read read, or %NULL.
 * @items_written: location to store number of bytes written or %NULL.
 *                 The value here stored does not include the trailing 0
 *                 byte. 
 * @error: location to store the error occuring, or %NULL to ignore
 *         errors. Any of the errors in #GConvertError other than
 *         %G_CONVERT_ERROR_NO_CONVERSION may occur.
 *
 * Convert a string from a 32-bit fixed width representation as UCS-4.
 * to UTF-8. The result will be terminated with a 0 byte.
 * 
 * Return value: a pointer to a newly allocated UTF-8 string.
 *               This value must be freed with g_free(). If an
 *               error occurs, %NULL will be returned and
 *               @error set.
 **/
char *
g_ucs4_to_utf8 (const wchar_t *str,
		long           len,              
		long          *items_read,       
		long          *items_written,    
		wchar_t       **error)
{
  int result_length;
  char *result = NULL;
  char *p;
  int i;

  result_length = 0;
  for (i = 0; len < 0 || i < len ; i++)
    {
      if (!str[i])
	break;

      if ((unsigned)str[i] >= 0x80000000)
	{
	  if (items_read)
	    *items_read = i;
          if (error)
              *error = L"Character out of range for UTF-8";
	  goto err_out;
	}
      
      result_length += UTF8_LENGTH (str[i]);
    }

  result = (char*)malloc (result_length + 1);
  p = result;

  i = 0;
  while (p < result + result_length)
    p += g_unichar_to_utf8 (str[i++], p);
  
  *p = '\0';

  if (items_written)
    *items_written = p - result;

 err_out:
  if (items_read)
    *items_read = i;

  return result;
}

std::string toUtf8(const std::wstring &str)
{
    long readed, writed;
    wchar_t *errMsg = NULL;
    
    char *res = g_ucs4_to_utf8(str.c_str(), str.length(), &readed,
            &writed, &errMsg);
    if (! res) {
        if (errMsg)
            throw Exception(errMsg);
        else
            throw Exception(L"Error converting text to UTF-8");
    }

    std::string s(res);
    free(res);

    return s;
}

std::wstring fromUtf8(const std::string &str)
{
    long readed, writed;
    wchar_t *errMsg = NULL;
    
    wchar_t *res = g_utf8_to_ucs4(str.c_str(), str.length(), &readed,
            &writed, &errMsg);
    if (! res) {
        if (errMsg)
            throw Exception(errMsg);
        else
            throw Exception(L"Error converting text from UTF-8");
    }

    std::wstring s(res);
    free(res);
    
    return s;
}


#else



std::string toUtf8(const std::wstring &str)
{
    if (! str.length())
        return "";
    
    int len = str.length();
    int bufSize = (len + 1) * 6 + 1;
    char buf[bufSize];
    int res = WideCharToMultiByte(CP_UTF8, 0, str.c_str(), len + 1,
           buf, bufSize, NULL, NULL);

    if (! res)
        throw Exception(L"Error converting UCS-2 to UTF-8");
    return buf;
}

std::wstring fromUtf8(const std::string &str)
{
    if (! str.length())
        return L"";
    
    int len = str.length();
    wchar_t buf[len + 1];

    int res = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), len + 1, 
            buf, len + 1);
    if (! res)
        throw Exception(L"Error converting UTF-8 to UCS-2");
    return buf;
}


std::string toOem(const std::wstring &str)
{
    if (! str.length())
        return "";
    
    int len = str.length();
    int bufSize = (len + 1) * 6 + 1;
    char buf[bufSize];
    int res = WideCharToMultiByte(CP_OEMCP, 0, str.c_str(), len + 1,
           buf, bufSize, NULL, NULL);

    if (! res)
        throw Exception(L"Error converting UCS-2 to OEM");
    return buf;
}

std::wstring fromOem(const std::string &str)
{
    if (! str.length())
        return L"";
    
    int len = str.length();
    wchar_t buf[len + 1];

    int res = MultiByteToWideChar(CP_OEMCP, 0, str.c_str(), len + 1, 
            buf, len + 1);
    if (! res)
        throw Exception(L"Error converting OEM to UCS-2");
    return buf;
}

#endif


std::wstring fromUtf8(const char *str, int len)
{
    char *buf = (char*)malloc(len + 1);
    if (! buf)
        throw Exception(L"Error allocating memory");
    memcpy(buf, str, len);
    buf[len] = 0;
    std::string s(buf);
    free(buf);
    return fromUtf8(s);
}


std::string toMbcs(const std::wstring &str)
{
    int len = str.length();
    if (! len)
        return "";
    else {
        int maxSize = MB_CUR_MAX * len;
        char buf[maxSize + 1];
        size_t l = wcstombs(buf, str.c_str(), maxSize);
        if ((size_t)-1 == -l) {         // convert what we can
            std::string res;
            for (int i = 0; i < len; i++) {
                int b = wctomb(buf, str[i]);
                if (0 < b) {
                    buf[b] = 0;
                    res += buf;
                }
            }
            return res;
        } else {
            buf[l] = 0;
            return buf;
        }
    }
}


std::wstring fromMbcs(const std::string &str)
{
    int maxLen = str.length();
    wchar_t ws[maxLen + 1];
    size_t cnt = mbstowcs(ws, str.c_str(), maxLen);
    if (cnt == (size_t)-1) {
        return L"";
    }
    ws[cnt] = 0;
    return ws;
}


std::ostream& operator << (std::ostream &stream, const std::wstring &str)
{
#ifdef WIN32
    if ((stream == std::cout) || (stream == std::cerr) || 
            (stream == std::clog))
        stream << toOem(str);
    else
#endif
    stream << toMbcs(str);
    return stream;
}


int getUtf8Length(unsigned char c)
{
    int mask, len;
    UTF8_COMPUTE(c, mask, len);
    if (-1 == len)
        throw Exception(L"Invalid utf-8 character");
    else
        return len;
}


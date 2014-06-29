/*****************************************************************************
 The MIT License

 Copyright (c) since 2009 Leandro T. C. Melo

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
*****************************************************************************/

#ifndef URL_HPP
#define URL_HPP

#include "config.hpp"
#include <string>
#include <iosfwd>

BUNDLE_NAMESPACE_BEGIN

/*
 * Class Url
 *
 * Represents an URL (Uniform Resource Locator). Based on the following RFCs:
 *  - RFC 3986 (obsoletes RFCs 2732, 2396, and 1808);
 *  - RFC 1738.
 */


class Url
{
public:
  Url(std::string const& representation);
  Url(std::string const& scheme,
      std::string const& host,
      std::string const& path,
      std::string const& query = "",
      std::string const& fragment = "");
  Url(std::string const& scheme,
      std::string const& host,
      int port,
      std::string const& path,
      std::string const& query = "",
      std::string const& fragment = "");
  Url(Url const& context, std::string const& representation);


  //Acessors and mutators.
  std::string const& get_scheme() const;
  std::string const& get_authority() const;
  std::string const& get_user_info() const;
  std::string const& get_host() const;
  int get_port() const;
  std::string const& get_path() const;
  std::string const& get_query() const;
  std::string const& get_fragment() const;

  std::string ToString() const;

private:

  class UrlParser
  {
  public:
    static void Execute(std::string const& representation,
                        std::string & scheme,
                        std::string & authority,
                        std::string & user_info,
                        std::string & host,
                        int & port,
                        std::string & path,
                        std::string & query,
                        std::string & fragment,
                        bool relative_resolution);

  private:
    static void ExtractScheme(std::string const& representation,
                              std::string & scheme,
                              std::size_t & pos,
                              bool relative_resolution);
    static void ExtractAuthority(std::string const& representation,
                                 std::string & authority,
                                 std::string & user_info,
                                 std::string & host,
                                 int & port,
                                 std::size_t & pos);
  };

  void ResolveRelativeness(Url const& context, std::string const& representation);
  void SetPathFromReferenceRemovingDotSegments(std::string & reference);
  std::string MergePathWithReference(std::string const& reference) const;
  void RemoveLastSegmentFromPath();
  void SetAuthority(std::string const& authority,
                    std::string const& user_info,
                    std::string const& host,
                    int port);

  //Constants for relative resolution.
  static const std::string sds;
  static const std::string sdds;
  //Cached to avoid a large number of repeated calls for the begin and end iterators.
  static const std::string::const_iterator sds_begin;
  static const std::string::const_iterator sds_end;
  static const std::string::const_iterator sdds_begin;
  static const std::string::const_iterator sdds_end;

  std::string scheme_; //Protocol.
  std::string authority_;
  std::string user_info_;
  std::string host_;
  int port_; //-1 indicates default port.
  std::string path_;
  std::string query_;
  std::string fragment_;
};

bool operator==(Url const&, Url const&);
bool operator!=(Url const&, Url const&);
std::ostream & operator<<(std::ostream &, Url const&);


NAMESPACE_END

#endif //URL_HPP

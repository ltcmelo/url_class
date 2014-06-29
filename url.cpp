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

#include "url.hpp"
#include "url_syntax_exception.hpp"
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <algorithm>

BUNDLE_NAMESPACE_BEGIN

const std::string Url::sds = "/./";
const std::string Url::sdds = "/../";
const std::string::const_iterator Url::sds_begin = Url::sds.begin();
const std::string::const_iterator Url::sds_end = Url::sds.end();
const std::string::const_iterator Url::sdds_begin = Url::sdds.begin();
const std::string::const_iterator Url::sdds_end = Url::sdds.end();


Url::Url(std::string const& representation) : port_(-1)
{
  UrlParser::Execute(representation,
    scheme_,
    authority_,
    user_info_,
    host_,
    port_,
    path_,
    query_,
    fragment_,
    false);
}

Url::Url(std::string const& scheme,
         std::string const& host,
         std::string const& path,
         std::string const& query,
         std::string const& fragment) :
  scheme_(scheme), authority_(host), host_(host), port_(-1), path_(path), query_(query),
  fragment_(fragment)
{
}

Url::Url(std::string const& scheme,
         std::string const& host,
         int port,
         std::string const& path,
         std::string const& query,
         std::string const& fragment) :
  scheme_(scheme), host_(host), port_(port), path_(path), query_(query), fragment_(fragment)
{
  std::stringstream ss;
  ss << port;
  authority_ = host + ":" + ss.str();
}

Url::Url(Url const& context, std::string const& representation) :
  scheme_(context.scheme_), authority_(context.authority_), user_info_(context.user_info_),
  host_(context.host_), port_(context.port_), path_(context.path_), query_(context.query_),
  fragment_(context.fragment_)
{
  if (representation.empty())
    return; //Simply inherit from context.

  this->ResolveRelativeness(context, representation);
}

std::string const&
Url::get_scheme() const
{
  return scheme_;
}

std::string const&
Url::get_authority() const
{
  return authority_;
}

std::string const&
Url::get_user_info() const
{
  return user_info_;
}

std::string const&
Url::get_host() const
{
  return host_;
}

int
Url::get_port() const
{
  return port_;
}

std::string const&
Url::get_path() const
{
  return path_;
}

std::string const&
Url::get_query() const
{
  return query_;
}

std::string const&
Url::get_fragment() const
{
  return fragment_;
}

std::string
Url::ToString() const
{
  std::stringstream ss;
  ss << *this;
  return ss.str();
}

void
Url::UrlParser::Execute(std::string const& representation,
                        std::string & scheme,
                        std::string & authority,
                        std::string & user_info,
                        std::string & host,
                        int & port,
                        std::string & path,
                        std::string & query,
                        std::string & fragment,
                        bool relative_resolution)
{
  std::size_t current_pos = 0;

  //Scheme...
  UrlParser::ExtractScheme(representation, scheme, current_pos, relative_resolution);

  //Authority...
  UrlParser::ExtractAuthority(representation, authority, user_info, host, port, current_pos);

  if (current_pos == std::string::npos)
    //In this case, the authority exists. However the URL has no path, query or fragment.
    return;

  //Path, query and fragment... (Notice that the initial slash is part of the path.)
  std::size_t question_pos = representation.find('?', current_pos);
  std::size_t square_pos = representation.find('#', current_pos);

  if (question_pos != std::string::npos)
  {
    path = representation.substr(current_pos, question_pos - current_pos);
    current_pos = question_pos + 1;
    query = representation.substr(current_pos, square_pos - current_pos); //If square_pos
      //is valid, the string is extracted as usual. Otherwise, just take it till the end anyway.
      //Since the fragment is the last possible part of a URL, this behavior would be OK.
  }
  else
    //The square_pos might be invalid here. However, for the same reason as above, taking the
    //string till the end would be OK.
    path = representation.substr(current_pos, square_pos - current_pos);

  if (square_pos != std::string::npos)
    fragment = representation.substr(square_pos + 1);
}

void
Url::UrlParser::ExtractScheme(std::string const& representation,
                              std::string & scheme,
                              std::size_t & current_pos,
                              bool relative_resolution)
{
  //A scheme may or may not exist in a relative reference. In addition, according to section 4.2
  //of RFC3986, the first path part of a relative reference may contain a colon. However, in this
  //case it must start with a dot-segment (so it's not mistaken for a scheme name).
  if (representation[0] == '.')
  {
    //There's no scheme to extract. Can only be a relative reference.
    if (!relative_resolution)
      throw UrlSyntaxException("Dot-segment preceding a scheme?");
    else
      return;
  }
  else
  {
    if ((current_pos = representation.find(':')) == std::string::npos)
      if (!relative_resolution)
        throw UrlSyntaxException("Scheme not found.");
      else
      {
        current_pos = 0;
        return;
      }
  }

  scheme = representation.substr(0, current_pos++);
  if (scheme.empty())
    throw UrlSyntaxException("Scheme is empty.");
}

void
Url::UrlParser::ExtractAuthority(std::string const& representation,
                                 std::string & authority,
                                 std::string & user_info,
                                 std::string & host,
                                 int & port,
                                 std::size_t & current_pos)
{
  //Depending on the scheme, an authority may or may not exist (both for absolute URLs or for
  //relative references). But when it exists, it's always preceded by the double-slash.
  if (representation[current_pos] != '/' || representation[current_pos + 1] != '/')
    //No authority. An URL like mailto:John.Doe@example.com or news:comp.lang.c++.
    return;

  std::size_t pos1 = current_pos + 2;

  //Try to find a separator. First using slash, then question mark, then square. If none is found,
  //take till the end of the string.
  if ((current_pos = representation.find('/', pos1)) == std::string::npos &&
      (current_pos = representation.find('?', pos1)) == std::string::npos)
    current_pos = representation.find('#', pos1);
  authority = representation.substr(pos1, current_pos - pos1);

  if (authority.empty())
    throw UrlSyntaxException("Authority is empty.");

  //User info, host and port...
  if ((pos1 = authority.find('@')) != std::string::npos)
    user_info = authority.substr(0, pos1);

  std::size_t pos2 = (pos1 == std::string::npos ? 0 : pos1 + 1); //0 or make it past the @.

  //If host is surrounded by square brackets, it's an IP-literal (probably IPv6).
  if (authority[pos2] == '[')
  {
    if ((pos1 = authority.find(']')) == std::string::npos)
      throw UrlSyntaxException("Unmatched square bracket in IP-literal.");

    if ((pos1 = authority.find(':', pos1)) != std::string::npos)
      port = atoi(authority.substr(pos1 + 1).c_str());
  }
  else
    if ((pos1 = authority.find(':', pos2)) != std::string::npos)
      port = atoi(authority.substr(pos1 + 1).c_str());

  host = authority.substr(pos2, pos1 - pos2);
}

void
Url::ResolveRelativeness(Url const& context, std::string const& representation)
{
  std::string scheme;
  std::string authority;
  std::string user_info;
  std::string host;
  int port = -1;
  std::string path;
  std::string query;
  std::string fragment;

  UrlParser::Execute(representation,
    scheme,
    authority,
    user_info,
    host,
    port,
    path,
    query,
    fragment,
    true);

  //This is the algorithm described in section 5.2.2 of RFC 3986. Except for when values are
  //inherited from the context (base) URL. In this case, they were already initialized in the
  //constructor.
  if (!scheme.empty())
  {
    scheme_ = scheme;
    this->SetAuthority(authority, user_info, host, port);
    this->SetPathFromReferenceRemovingDotSegments(path);
    query_ = query;
  }
  else
  {
    if (!authority.empty())
    {
      this->SetAuthority(authority, user_info, host, port);
      this->SetPathFromReferenceRemovingDotSegments(path);
      query_ = query;
    }
    else
    {
      if (path.empty())
      {
        if (!query.empty())
          query_ = query;
      }
      else
      {
        if (path[0] == '/')
          this->SetPathFromReferenceRemovingDotSegments(path);
        else
        {
          std::string merged_path = this->MergePathWithReference(path);
          this->SetPathFromReferenceRemovingDotSegments(merged_path);
        }
        query_ = query;
      }
    }
  }
  fragment_ = fragment;
}

void
Url::SetPathFromReferenceRemovingDotSegments(std::string & reference)
{
  path_.clear(); //Original path is cleared.

  if (reference.empty())
    return;

  //In the search process for specific path segments, I could have used std::string::find with a
  //check against the current position: if (reference.find("../") == current_pos)
  //However, this would require a pass through the whole string (which could be a relatively long
  //URL) for every mismatch. Therefore, I find the strategy below a better approach.

  std::size_t size = reference.size(); //To avoid repeated calls.
  std::string::iterator last = reference.end();
  std::string::iterator current = reference.begin();
  while (current != last)
  {
    std::iterator_traits<std::string::iterator>::difference_type remaining =
      std::distance(current, last);

    //Search for ./
    if (remaining >= 2 &&
        std::search(current, current + 2, sds_begin + 1, sds_end) == current)
      std::advance(current, 2);
    //Search for ../
    else if (remaining >= 3 &&
             std::search(current, current + 3, sdds_begin + 1, sdds_end) == current)
      std::advance(current, 3);
    //Search for /../
    else if (remaining >= 4 &&
             std::search(current, current + 4, sdds_begin, sdds_end) == current)
    {
      this->RemoveLastSegmentFromPath();
      std::advance(current, 3);
    }
    //Search for /..
    else if (remaining == 3 &&
             std::search(current, current + 3, sdds_begin, sdds_begin + 3) == current)
    {
      this->RemoveLastSegmentFromPath();
      *(current + 2) = '/';
      std::advance(current, 2);
    }
    //Search for /./
    else if (remaining >= 3 &&
             std::search(current, current + 3, sds_begin, sds_end) == current)
      std::advance(current, 2);
    //Search for /.
    else if (remaining == 2 &&
             std::search(current, current + 2, sds_begin, sds_begin + 2) == current)
    {
      *(current + 1) = '/';
      ++current;
    }
    //Check for ..
    else if (remaining == 2 && *current == '.' && *(current + 1) == '.')
      std::advance(current, 2);
    //Check for .
    else if (remaining == 1 && *current == '.')
      ++current;
    else
    {
      std::size_t current_pos = size - remaining;

      //The C++ standard makes no restrictions on the second argument of std::string::find.
      //So even if current_pos is the last character of the string, adding 1 and using it as
      //below should just generate and std::string::npos return (no out of range exception).
      std::size_t next_pos = reference.find('/', static_cast<std::size_t>(current_pos + 1));
      path_ += reference.substr(current_pos, next_pos - current_pos);//Either till the next slash
        //or till the end of the string.
      if (next_pos == std::string::npos)
        break;
      std::advance(current, next_pos - current_pos);
    }
  }
}

void
Url::RemoveLastSegmentFromPath()
{
  std::size_t pos = path_.rfind('/');
  if (pos != std::string::npos)
    path_.erase(pos);
}

std::string
Url::MergePathWithReference(std::string const& reference) const
{
  //If there's an authority, the path follows an hierarchical form. In this case, if the path is
  //empty, just concatenate a slash with the reference.
  if (!authority_.empty() && path_.empty())
    return std::string("/" + reference);

  std::size_t pos = path_.find_last_of('/');
  if (pos == std::string::npos)
    //Merge consists of only the reference. The entire path_ is excluded when no slash is found.
    return reference;
  else
  {
    std::string merged(path_.substr(0, pos + 1)); //Keep the slash.
    return merged + reference;
  }
}

void
Url::SetAuthority(std::string const& authority,
                  std::string const& user_info,
                  std::string const& host,
                  int port)
{
  authority_ = authority;
  user_info_ = user_info;
  host_ = host;
  port_ = port;
}

bool operator==(Url const& one, Url const& other)
{
  return one.get_scheme() == other.get_scheme() &&
    one.get_authority() == other.get_authority() &&
    one.get_user_info() == other.get_user_info() &&
    one.get_host() == other.get_host() &&
    one.get_port() == other.get_port() &&
    one.get_path() == other.get_path() &&
    one.get_query() == other.get_query();
    //Fragment is not taken into consideration.
}

bool operator!=(Url const& one, Url const& other)
{
  return !(one == other);
}

std::ostream & operator<<(std::ostream & out, Url const& url)
{
  out << url.get_scheme() << ":";
  if (!url.get_authority().empty())
    out << "//" << url.get_authority();
  out << url.get_path();
  if (!url.get_query().empty())
    out << "?" << url.get_query();
  if (!url.get_fragment().empty())
    out << "#" << url.get_fragment();
  return out;
}

NAMESPACE_END

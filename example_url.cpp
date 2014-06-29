#include <iostream>
#include "url.hpp"

void PrintComponents(bundle::Url const& url)
{
  std::cout << "------> " << url << std::endl;
  std::cout << "scheme: " << url.get_scheme() << std::endl;
  std::cout << "authority: " << url.get_authority() << std::endl;
  std::cout << "user-info: " <<url.get_user_info() << std::endl;
  std::cout << "host: " << url.get_host() << std::endl;
  std::cout << "port: " << url.get_port() << std::endl; //-1 indicates no port specified.
  std::cout << "path: " << url.get_path() << std::endl;
  std::cout << "query: " << url.get_query() << std::endl;
  std::cout << "fragment: " << url.get_fragment() << std::endl;
}

int main()
{
  bundle::Url url0("http://www.bla.com:8080/p/a/t/h?q=y#f");
  PrintComponents(url0);

  //Relative.
  bundle::Url url1(url0, "../w/");
  PrintComponents(url1);

  bundle::Url url2("news:comp.lanc.c++");
  PrintComponents(url2);

  //Also check the ToString() method.

  return 0;
}

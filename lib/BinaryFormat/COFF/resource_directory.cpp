// resource_directory.cpp:

#include "resource_directory.h"

#include <algorithm>
#include <cstring>
#include <cwchar>
#include <iostream>
#include <iomanip>
#include <locale>
#include <memory>
#include <string>
#include <vector>

#ifndef _WIN32
#include <iconv.h>
#endif

using namespace binlab::COFF;

std::ostream& binlab::COFF::operator<<(std::ostream& os, const IMAGE_RESOURCE_DIR_STRING_U& string) {
#ifdef _WIN32
  using char_type = std::decay_t<decltype(string.NameString[0])>;
  std::locale loc;
  std::string name(string.Length, '\0');

  std::use_facet<std::ctype<char_type>>(loc).narrow(string.NameString, string.NameString + string.Length, '.', name.data());
  os << name;
#else
  auto cd = iconv_open("UTF-8", "UTF-16LE");
  if (cd == reinterpret_cast<iconv_t>(-1)) {
    throw std::system_error{errno, std::system_category(), "iconv_open"};
  }

  char buff[1024];
  auto inbuf = const_cast<char*>(reinterpret_cast<const char*>(string.NameString));
  std::size_t inbytesleft = string.Length * sizeof(string.NameString[0]);
  auto outbuf = buff;
  std::size_t outbytesleft = sizeof(buff);
  if (iconv(cd, &inbuf, &inbytesleft, &outbuf, &outbytesleft) == static_cast<std::size_t>(-1)) {
    throw std::system_error{errno, std::system_category(), "iconv"};
  }
  os.write(buff, sizeof(buff) - outbytesleft);

  if (iconv_close(cd) == -1) {
    throw std::system_error{errno, std::system_category(), "iconv_close"};
  }
#endif  // _WIN32
  return os;
}

class resource_dir_string_iterator {
 public:
  using iterator_category   = std::forward_iterator_tag;
  using value_type          = IMAGE_RESOURCE_DIR_STRING_U;
  using reference           = std::add_lvalue_reference_t<value_type>;
  using pointer             = std::add_pointer_t<value_type>;

  resource_dir_string_iterator(void* ptr) : ptr_{static_cast<char*>(ptr)} {}

  [[nodiscard]] reference operator*() noexcept { return reinterpret_cast<reference>(*ptr_); }
  [[nodiscard]] pointer operator->() noexcept { return std::pointer_traits<pointer>::pointer_to(**this); }

  resource_dir_string_iterator& operator++() noexcept {
    ptr_ += sizeof((*this)->Length) + (*this)->Length * sizeof((*this)->NameString[0]);
    return *this;
  }

  [[nodiscard]] resource_dir_string_iterator operator++(int) noexcept {
    auto tmp = *this;
    ++(*this);
    return tmp;
  }

 private:
  char* ptr_;
};

std::ostream& binlab::COFF::operator<<(std::ostream& os, const IMAGE_RESOURCE_DIRECTORY& directory) {
  os << std::setw(8) << directory.Characteristics;
  os << std::setw(8) << directory.TimeDateStamp;
  os << std::setw(8) << directory.MajorVersion;
  os << std::setw(8) << directory.MinorVersion;
  os << std::setw(8) << directory.NumberOfNamedEntries;
  os << std::setw(8) << directory.NumberOfIdEntries;
  return os;
}

std::ostream& binlab::COFF::operator<<(std::ostream& os, const IMAGE_RESOURCE_DIRECTORY_ENTRY& entry) {
  os << std::setw(8) << entry.NameOffset;
  os << std::setw(9) << entry.OffsetToData;
  os << std::setw(8) << entry.OffsetToDirectory;
  return os;
}

std::ostream& binlab::COFF::operator<<(std::ostream& os, const IMAGE_RESOURCE_DATA_ENTRY& entry) {
  os << std::setw(8) << entry.OffsetToData;
  os << std::setw(8) << entry.Size;
  os << std::setw(8) << entry.CodePage;
  return os;
}

std::ostream& binlab::COFF::dump(std::ostream& os, char* base, std::size_t vbase, IMAGE_RESOURCE_DIRECTORY* directory, std::size_t depth) {
  os << std::setw((depth + 1) * 2) << ' ';
  //os << std::setw(10) << "res dir:" << *directory << '\n';
  auto entry = begin(*directory);
  for (std::size_t i = 0; i < directory->NumberOfIdEntries + directory->NumberOfNamedEntries; ++i) {
    os << std::setw((depth + 1) * 2) << ' ';
    //os << std::setw(10) << "res ent:" << entry[i];
    if (entry[i].NameIsString) {
      os << reinterpret_cast<IMAGE_RESOURCE_DIR_STRING_U&>(base[entry[i].NameOffset]);
    } else {
      //os << "ID: " << std::setw(4);
      os << entry[i].Id;
    }
    os << '\n';

    if (entry[i].DataIsDirectory) {
      dump(os, base, vbase, reinterpret_cast<IMAGE_RESOURCE_DIRECTORY*>(&base[entry[i].OffsetToDirectory]), depth + 1);
    } else {
      //auto& data = reinterpret_cast<IMAGE_RESOURCE_DATA_ENTRY&>(base[entry[i].OffsetToData]);
    }
  }
  return os;
}

std::ostream& dump_rt_string(std::ostream& os, IMAGE_RESOURCE_DATA_ENTRY& data, char* base) {
  std::vector<char> content(data.Size);
  resource_dir_string_iterator iter{base}, d_iter{&content[0]};
  for (std::size_t i = 0; i < 0x10; ++i, ++iter, ++d_iter) {
    std::pair<std::wstring, std::wstring> item{L"http://www.bing.cn/", L"http://www.baidu.cn/"};
    std::wstring name(iter->NameString, iter->NameString + iter->Length);
    if (name == item.first) {
      d_iter->Length = static_cast<WORD>(item.second.size());
      std::ranges::copy(item.second, std::addressof(d_iter->NameString[0]));
      //std::wcout << L"replace(RT_STRING) " << item.first << L" to " << item.second << L'\n';
      if (item.first.size() < item.second.size()) {
        throw std::runtime_error{"target string too long"};
      }
    } else {
      std::copy_n(reinterpret_cast<char*>(std::addressof(*iter)), (iter->Length + 1) * sizeof(iter->NameString[0]), reinterpret_cast<char*>(std::addressof(*d_iter)));
    }
  }
  data.Size = static_cast<DWORD>(std::distance(&content[0], reinterpret_cast<char*>(std::addressof(*d_iter))));
  std::memcpy(base, &content[0], data.Size);
  return os;
}

struct DLGTEMPLATE {
  DWORD style;
  DWORD dwExtendedStyle;
  WORD  cdit;
  short x;
  short y;
  short cx;
  short cy;
};

struct DLGITEMTEMPLATE {
  DWORD style;
  DWORD dwExtendedStyle;
  short x;
  short y;
  short cx;
  short cy;
  WORD  id;
};

struct DLGTEMPLATEEX {
  WORD      dlgVer;
  WORD      signature;
  DWORD     helpID;
  DWORD     exStyle;
  DWORD     style;
  WORD      cDlgItems;
  short     x;
  short     y;
  short     cx;
  short     cy;
  //sz_Or_Ord menu;
  //sz_Or_Ord windowClass;
  //WCHAR     title[titleLen];
  //WORD      pointsize;
  //WORD      weight;
  //BYTE      italic;
  //BYTE      charset;
  //WCHAR     typeface[stringLen];
};

struct DLGTEMPLATEEX2 {
  //WORD      dlgVer;
  //WORD      signature;
  //DWORD     helpID;
  //DWORD     exStyle;
  //DWORD     style;
  //WORD      cDlgItems;
  //short     x;
  //short     y;
  //short     cx;
  //short     cy;
  //sz_Or_Ord menu;
  //sz_Or_Ord windowClass;
  //WCHAR     title[titleLen];
  WORD      pointsize;
  WORD      weight;
  BYTE      italic;
  BYTE      charset;
  //WCHAR     typeface[stringLen];
};

typedef struct {
  DWORD     helpID;
  DWORD     exStyle;
  DWORD     style;
  short     x;
  short     y;
  short     cx;
  short     cy;
  DWORD     id;
  //sz_Or_Ord windowClass;
  //sz_Or_Ord title;
  //WORD      extraCount;
} DLGITEMTEMPLATEEX;

std::ostream& operator<<(std::ostream& os, const DLGTEMPLATE& dialog) {
  os << std::setw(8) << dialog.style;
  os << std::setw(8) << dialog.dwExtendedStyle;
  os << std::setw(8) << dialog.cdit;
  os << std::setw(4) << dialog.x;
  os << std::setw(4) << dialog.y;
  os << std::setw(4) << dialog.cx;
  os << std::setw(4) << dialog.cy;
  return os;
}

std::ostream& operator<<(std::ostream& os, const DLGTEMPLATEEX& dialog) {
  os << std::setw(8) << dialog.dlgVer;
  os << std::setw(8) << dialog.signature;
  os << std::setw(8) << dialog.helpID;
  os << std::setw(8) << dialog.exStyle;
  os << std::setw(8) << dialog.style;
  os << std::setw(8) << dialog.cDlgItems;
  os << std::setw(4) << dialog.x;
  os << std::setw(4) << dialog.y;
  os << std::setw(4) << dialog.cx;
  os << std::setw(4) << dialog.cy;
  return os;
}

// http://bytepointer.com/resources/win32_res_format.htm
std::ostream& dump_rt_dialog(std::ostream& os, IMAGE_RESOURCE_DATA_ENTRY& data, char* base) {
  std::size_t pos = 0;
  wchar_t* str;

  auto& dialogex = reinterpret_cast<DLGTEMPLATEEX&>(base[pos]);
  if (dialogex.dlgVer != 1 || dialogex.signature != 0xffff) {
    auto& dialog = reinterpret_cast<DLGTEMPLATE&>(base[pos]);
    pos += sizeof(dialog);

    str = reinterpret_cast<wchar_t*>(&base[pos]);
    if (*str == 0xffff) {
      pos += 4;
    } else if (*str) {
      std::wstring menu(str);
      std::wcout << menu << L'\n';
      pos += menu.size() * 2;
      pos += 2;
    } else {
      pos += 2;
    }

    str = reinterpret_cast<wchar_t*>(&base[pos]);
    if (*str == 0xffff) {
      pos += 4;
    } else if (*str) {
      std::wstring windowClass(str);
      std::wcout << windowClass << L'\n';
      pos += windowClass.size() * 2;
      pos += 2;
    } else {
      pos += 2;
    }

    auto pointsize = reinterpret_cast<WORD&>(base[pos]);
    pos += 2;

    str = reinterpret_cast<wchar_t*>(&base[pos]);
    if (*str == 0xffff) {
      pos += 4;
    } else if (*str) {
      std::wstring typeface(str);
      std::wcout << typeface << L'\n';
      pos += typeface.size() * 2;
      pos += 2;
    } else {
      pos += 2;
    }

    for (std::size_t i = 0; i < dialog.cdit; ++i) {
      auto pad = pos % 4;  // align
      pos += pad;

      auto& item = reinterpret_cast<DLGITEMTEMPLATE&>(base[pos]);
      pos += sizeof(DLGITEMTEMPLATE);

      str = reinterpret_cast<wchar_t*>(&base[pos]);
      if (*str == 0xffff) {
        pos += 4;
      } else if (*str) {
        std::wstring windowClass(str);
        std::wcout << windowClass << L'\n';
        pos += windowClass.size() * 2;
        pos += 2;
      } else {
        pos += 2;
      }

      str = reinterpret_cast<wchar_t*>(&base[pos]);
      if (*str == 0xffff) {
        pos += 4;
      } else if (*str) {
        std::wstring title(str);
        std::wcout << title << L'\n';
        pos += title.size() * 2;
        pos += 2;
      } else {
        pos += 2;
      }
    }
    return os;
  }
  pos += sizeof(dialogex);
  
  str = reinterpret_cast<wchar_t*>(&base[pos]);
  if (*str) {
    std::wstring menu(str);
    std::wcout << menu << L'\n';
    pos += menu.size() * 2;
  } else {
    pos += 2;
  }

  str = reinterpret_cast<wchar_t*>(&base[pos]);
  if (*str) {
    std::wstring windowClass(str);
    std::wcout << windowClass << L'\n';
    pos += windowClass.size() * 2;
  } else {
    pos += 2;
  }

  str = reinterpret_cast<wchar_t*>(&base[pos]);
  if (*str) {
    std::wstring title(str);
    std::wcout << title << L'\n';
    pos += title.size() * 2;
  } else {
    pos += 2;
  }

  auto& fonts = reinterpret_cast<DLGTEMPLATEEX2&>(base[pos]);
  pos += sizeof(DLGTEMPLATEEX2);

  str = reinterpret_cast<wchar_t*>(&base[pos]);
  if (*str) {
    std::wstring typeface(str);
    std::wcout << typeface << L'\n';
    pos += typeface.size() * 2;
    pos += 2;
  } else {
    pos += 2;
  }

  for (std::size_t i = 0; i < dialogex.cDlgItems; ++i) {
    auto pad = pos % 4;  // align
    pos += pad;

    auto& item = reinterpret_cast<DLGITEMTEMPLATEEX&>(base[pos]);
    pos += sizeof(DLGITEMTEMPLATEEX);

    str = reinterpret_cast<wchar_t*>(&base[pos]);
    if (*str == 0xffff) {
      pos += 4;
    } else if (*str) {
      std::wstring windowClass(str);
      std::wcout << windowClass << L'\n';
      pos += windowClass.size() * 2;
      pos += 2;
    } else {
      pos += 2;
    }

    str = reinterpret_cast<wchar_t*>(&base[pos]);
    if (*str == 0xffff) {
      pos += 4;
    } else if (*str) {
      std::wstring title(str);
      std::wcout << title << L'\n';
      pos += title.size() * 2;
      pos += 2;
    } else {
      pos += 2;
    }

    auto extraCount = reinterpret_cast<WORD&>(base[pos]);
    //if (extraCount) {
    //  throw std::runtime_error{"extra count for DLGITEMTEMPLATEEX is non-zero"};
    //}
    pos += sizeof(extraCount);
  }
  return os;
}

//std::ostream& dump_rt_dialog(std::ostream& os, IMAGE_RESOURCE_DATA_ENTRY& data, char* base) {
//  std::pair<std::wstring, std::wstring> item{L"asdasdfasd", L"asdfoasjdfoasjfodsa"};
//
//  auto last1 = base + data.Size;
//  std::vector<char> content(data.Size, '\0');
//  auto iter = &content[0];
//
//  auto first2 = reinterpret_cast<const char*>(&item.first[0]);
//  auto last2 = first2 + item.first.size() * sizeof(item.first[0]);
//  auto first3 = reinterpret_cast<const char*>(&item.second[0]);
//  auto last3 = first3 + item.second.size() * sizeof(item.second[0]);
//
//  for (char *ptr1 = base, *ptr2;; ptr1 = ptr2) {
//    ptr2 = std::search(ptr1, last1, first2, last2);
//    iter = std::copy(ptr1, ptr2, iter);
//    if (ptr2 == last1) {
//      break;
//    }
//
//    // new replace
//    //std::wcout << L"replace(RT_DIALOG) " << token.first << L" to " << token.second << L'\n';
//    if (item.first.size() < item.second.size()) {
//      throw std::runtime_error{"target string too long"};
//    }
//    iter = std::copy(first3, last3, iter);
//    ptr2 += std::distance(first2, last2);
//
//    // add pad bytes
//    auto length = std::wcslen(reinterpret_cast<wchar_t*>(ptr2)) * sizeof(wchar_t);
//    iter = std::copy(ptr2, ptr2 + length, iter);
//    ptr2 += length;
//
//    auto pad = reinterpret_cast<std::size_t>(iter) % 4;
//    if (pad) {
//      os << "pad count: " << pad << '\n';
//      iter = std::fill_n(iter, pad, '\0');
//    }
//  }
//  data.Size = static_cast<DWORD>(std::distance(&content[0], iter));
//  std::memcpy(base, &content[0], data.Size);
//  return os;
//}

std::ostream& binlab::COFF::dump(std::ostream& os, IMAGE_SECTION_HEADER& section, char* base, IMAGE_RESOURCE_DIRECTORY& directory1) {
  auto entry1 = begin(directory1);
  for (std::size_t i = 0; i < directory1.NumberOfIdEntries + directory1.NumberOfNamedEntries; ++i) {
    //if (entry[i].NameIsString) {
    //  throw std::runtime_error{"resource directory entry name is string"};
    //}
    //os << entry[i] << '\n';

    if (!entry1[i].DataIsDirectory) {
      throw std::runtime_error{"2nd level resource directory entry for dialogs is not directory"};
    }

    auto& directory2 = reinterpret_cast<IMAGE_RESOURCE_DIRECTORY&>(base[entry1[i].OffsetToDirectory]);
    auto entry2 = begin(directory2);
    for (std::size_t j = 0; j < directory2.NumberOfIdEntries + directory2.NumberOfNamedEntries; ++j) {
      //if (entry2[j].NameIsString) {
      //  throw std::runtime_error{"2nd level resource directory entry name is string"};
      //}

      if (!entry2[j].DataIsDirectory) {
        throw std::runtime_error{"2nd level resource directory entry for string table is not directory"};
      }

      auto& directory3 = reinterpret_cast<IMAGE_RESOURCE_DIRECTORY&>(base[entry2[j].OffsetToDirectory]);
      auto entry3 = begin(directory3);
      for (std::size_t k = 0; k < directory3.NumberOfIdEntries + directory3.NumberOfNamedEntries; ++k) {
        if (entry3[k].NameIsString) {
          throw std::runtime_error{"3rd level resource directory entry name is string"};
        }

        if (entry3[k].DataIsDirectory) {
          throw std::runtime_error{"3rd level resource directory entry for string table is not data"};
        }

        // dump data
        auto& data_entry = reinterpret_cast<IMAGE_RESOURCE_DATA_ENTRY&>(base[entry3[k].OffsetToData]);
        auto data = &base[data_entry.OffsetToData - section.VirtualAddress];
        switch (entry1[i].Id) {
          case 5:  // RT_DIALOG
            dump_rt_dialog(os, data_entry, data);
            break;
          case 6:  // RT_STRING
          {
            //dump_rt_string(os, data_entry, data);
            //std::size_t l = 0;
            //for (resource_dir_string_iterator iter{data}; l < 0x10; ++l, ++iter) {
            //  if (!iter->Length) {
            //    continue;
            //  }
            //  os << "ID: " << std::setw(4) << ((entry2[j].Id - 1) * 0x10 + l) << ": " << *iter << '\n';
            //}
            break;
          }
          default:
            break;
        }
      }
    }
  }
  return os;
}

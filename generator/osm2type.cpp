#include "osm2type.hpp"
#include "xml_element.hpp"

#include "../indexer/classificator.hpp"
#include "../indexer/feature_visibility.hpp"

#include "../base/assert.hpp"
#include "../base/string_utils.hpp"
#include "../base/math.hpp"

#include "../std/vector.hpp"
#include "../std/bind.hpp"

#include <QtCore/QString>


namespace ftype
{
  namespace
  {
    /// get value of mark (1 == "yes", -1 == "no", 0 == not a "yes\no")
    static int get_mark_value(string const & k, string const & v)
    {
      static char const * aTrue[] = { "yes", "true", "1", "*" };
      static char const * aFalse[] = { "no", "false", "-1" };

      strings::SimpleTokenizer it(v, "|");
      while (it)
      {
        if (strings::IsInArray(aTrue, *it))
          return 1;

        if (k != "layer" && k != "oneway" && strings::IsInArray(aFalse, *it))
          return -1;

        ++it;
      }

      // "~" means no this tag, so sometimes it means true,
      // and all other cases - false. Choose according to a key.
      if (v == "~")
        return (k == "access" ? 1 : -1);

      return 0;
    }

    bool is_skip_tag(string const & k)
    {
      return (k == "created_by"
           || k == "description"
           || k == "cycleway"     // [highway=primary][cycleway=lane] parsed as [highway=cycleway]
           || k == "proposed"     // [highway=proposed][proposed=primary] parsed as [highway=primary]
           || k == "construction" // [highway=primary][construction=primary] parsed as [highway=construction]
           );
    }

    template <class ToDo>
    typename ToDo::result_type for_each_tag(XMLElement * p, ToDo toDo)
    {
      typedef typename ToDo::result_type res_t;

      res_t res = res_t();
      for (size_t i = 0; i < p->childs.size(); ++i)
      {
        if (p->childs[i].name == "tag")
        {
          string & k = p->childs[i].attrs["k"];
          string & v = p->childs[i].attrs["v"];

          if (k.empty() || is_skip_tag(k))
            continue;

          // this means "no"
          if (get_mark_value(k, v) == -1)
            continue;

          res = toDo(k, v);
          if (res)
            return res;
        }
      }
      return res;
    }

    bool is_name_tag(string const & k)
    {
      return (string::npos != k.find("name"));
    }

    class do_print
    {
      ostream & m_s;
    public:
      typedef bool result_type;

      do_print(ostream & s) : m_s(s) {}
      bool operator() (string const & k, string const & v) const
      {
        m_s << k << " <---> " << v << endl;
        return false;
      }
    };

    class do_find_name
    {
      set<string> m_savedNames;

      size_t & m_count;
      FeatureParams & m_params;

    public:
      typedef bool result_type;

      do_find_name(size_t & count, FeatureParams & params)
        : m_count(count), m_params(params)
      {
        m_count = 0;
      }

      bool GetLangByKey(string const & k, string & lang)
      {
        strings::SimpleTokenizer token(k, "\t :");
        if (!token)
          return false;

        // this is an international (latin) name
        if (*token == "int_name")
          lang = "int_name";
        else
        {
          if (*token == "name")
          {
            ++token;
            lang = (token ? *token : "default");

            // replace dummy arabian tag with correct tag
            if (lang == "ar1")
              lang = "ar";
          }
        }

        if (lang.empty())
          return false;

        // avoid duplicating names
        return m_savedNames.insert(lang).second;
      }

      bool operator() (string const & k, string const & v)
      {
        ++m_count;

        if (v.empty())
          return false;

        // get name with language suffix
        string lang;
        if (GetLangByKey(k, lang))
        {
          // Unicode Compatibility Decomposition,
          // followed by Canonical Composition (NFKC).
          // Needed for better search matching
          QByteArray const normBytes = QString::fromUtf8(
                v.c_str()).normalized(QString::NormalizationForm_KC).toUtf8();
          m_params.AddName(lang, normBytes.constData());
        }

        // get layer
        if (k == "layer" && m_params.layer == 0)
        {
          m_params.layer = atoi(v.c_str());
          int8_t const bound = 10;
          m_params.layer = my::clamp(m_params.layer, -bound, bound);
        }

        // get reference (we process road numbers only)
        if (k == "ref" && v != "route")
          m_params.ref = v;

        // get house number
        if (k == "addr:housenumber")
        {
          // Treat "numbers" like names if it's not an actual number.
          if (!m_params.AddHouseNumber(v))
            m_params.AddHouseName(v);
        }
        if (k == "addr:housename")
          m_params.AddHouseName(v);
        if (k == "addr:street")
          m_params.AddStreetAddress(v);
        if (k == "addr:flats")
          m_params.flats = v;

        // get population rank
        if (k == "population")
        {
          uint64_t n;
          if (strings::to_uint64(v, n))
            m_params.rank = static_cast<uint8_t>(log(double(n)) / log(1.1));
        }

        return false;
      }
    };

    class do_find_obj
    {
      ClassifObject const * m_parent;
      bool m_isKey;

      bool is_good_tag(string const & k, string const & v) const
      {
        if (is_name_tag(k))
          return false;

        // Filter 3rd component of type here.
        if (m_isKey)
        {
          /// @todo Probably, we need to filter most keys like == "yes" here,
          /// but need to carefully investigate the classificator.

          // Grab only "capital == yes" and skip all other capitals.
          if (k == "capital")
            return (get_mark_value(k, v) == 1);
        }
        else
        {
          // Numbers are used in boundary-administrative-X types.
          // Take only "admin_level" tags to avoid grabbing any other trash numbers.
          uint64_t dummy;
          if (strings::to_uint64(v, dummy))
            return (k == "admin_level");
        }

        return true;
      }

    public:
      typedef ClassifObjectPtr result_type;

      do_find_obj(ClassifObject const * p, bool isKey) : m_parent(p), m_isKey(isKey) {}

      ClassifObjectPtr operator() (string const & k, string const & v) const
      {
        if (is_good_tag(k, v))
        {
          ClassifObjectPtr p = m_parent->BinaryFind(m_isKey ? k : v);
          if (p)
            return p;
        }
        return ClassifObjectPtr(0, 0);
      }
    };

    typedef vector<ClassifObjectPtr> path_type;

    class do_find_root_obj
    {
      set<int> & m_skipTags;
      path_type & m_path;
      int m_id;

    public:
      typedef bool result_type;

      do_find_root_obj(set<int> & skipTags, path_type & path)
        : m_skipTags(skipTags), m_path(path), m_id(0)
      {
      }

      bool operator() (string const & k, string const & v)
      {
        if (m_skipTags.count(m_id) == 0)
        {
          // first try to match key
          ClassifObjectPtr p = do_find_obj(classif().GetRoot(), true)(k, v);
          if (p)
          {
            m_path.push_back(p);

            // now try to match correspondent value
            p = do_find_obj(p.get(), false)(k, v);
            if (p)
              m_path.push_back(p);

            m_skipTags.insert(m_id);
            return true;
          }
        }

        ++m_id;
        return false;
      }
    };

    /// Process synonym tags to match existing classificator types.
    /// @todo We are planning to rewrite classificator <-> tags matching.
    class do_replace_synonyms
    {
    public:
      typedef bool result_type;

      bool operator() (string & k, string & v) const
      {
        if (v == "yes")
        {
          if (k == "atm" || k == "restaurant")
          {
            k.swap(v);
            k = "amenity";
          }
          else if (k == "hotel")
          {
            k.swap(v);
            k = "tourism";
          }
        }

        return false;
      }
    };

    class do_find_pairs
    {
      char const * (*m_arr) [2];
      size_t m_size;

      vector<bool> m_res;

    public:
      do_find_pairs(char const * (*arr) [2], size_t sz)
        : m_arr(arr), m_size(sz)
      {
        m_res.resize(m_size, false);
      }

      bool operator() (string const & k, string const & v)
      {
        for (size_t i = 0; i < m_size; ++i)
          if ((k == "*" || k == m_arr[i][0]) &&
              (v == "*" || v == m_arr[i][1]))
          {
            m_res[i] = true;
          }

        return false;
      }

      bool IsExist(size_t i) const { return m_res[i]; }
    };
  }

  ClassifObjectPtr find_object(ClassifObject const * parent, XMLElement * p, bool isKey)
  {
    return for_each_tag(p, do_find_obj(parent, isKey));
  }

  size_t process_common_params(XMLElement * p, FeatureParams & params)
  {
    size_t count;
    for_each_tag(p, do_find_name(count, params));
    return count;
  }

  void process_synonims(XMLElement * p)
  {
    for_each_tag(p, do_replace_synonyms());
  }

  void add_layers(XMLElement * p)
  {
    static char const * arr[][2] = {
      { "bridge", "yes" },
      { "tunnel", "yes" },
      { "layer", "*" },
    };

    do_find_pairs doFind(arr, ARRAY_SIZE(arr));
    for_each_tag(p, bind<bool>(ref(doFind), _1, _2));

    if (!doFind.IsExist(2))
    {
      if (doFind.IsExist(0))
        p->AddKV("layer", "1");
      if (doFind.IsExist(1))
        p->AddKV("layer", "-1");
    }
  }

//#ifdef DEBUG
//  class debug_find_string
//  {
//    string m_comp;
//  public:
//    debug_find_string(string const & comp) : m_comp(comp) {}
//    typedef bool result_type;
//    bool operator() (string const & k, string const & v) const
//    {
//      return (k == m_comp || v == m_comp);
//    }
//  };
//#endif

  class CachedTypes
  {
    buffer_vector<uint32_t, 16> m_types;

  public:
    enum Type { ENTRANCE, HIGHWAY, ADDRESS, ONEWAY, PRIVATE, LIT };

    CachedTypes()
    {
      Classificator const & c = classif();

      char const * arr1[][1] = {
        { "entrance" },
        { "highway" }
      };
      for (size_t i = 0; i < ARRAY_SIZE(arr1); ++i)
        m_types.push_back(c.GetTypeByPath(vector<string>(arr1[i], arr1[i] + 1)));

      char const * arr2[][2] = {
        { "building", "address" },
        { "hwtag", "oneway" },
        { "hwtag", "private" },
        { "hwtag", "lit" },
      };
      for (size_t i = 0; i < ARRAY_SIZE(arr2); ++i)
        m_types.push_back(c.GetTypeByPath(vector<string>(arr2[i], arr2[i] + 2)));
    }

    uint32_t Get(Type t) const { return m_types[t]; }
    bool IsHighway(uint32_t t) const
    {
      ftype::TruncValue(t, 1);
      return t == Get(HIGHWAY);
    }
  };

  void GetNameAndType(XMLElement * p, FeatureParams & params)
  {
    process_synonims(p);
    add_layers(p);

    // maybe an empty feature
    if (process_common_params(p, params) == 0)
      return;

    set<int> skipRootKeys;
    do
    {
      path_type path;

      // find first root object by key
      do_find_root_obj doFindRoot(skipRootKeys, path);
      for_each_tag(p, doFindRoot);

      if (path.empty())
        break;

      // continue find path from last element
      do
      {
        // next objects trying to find by value first
        ClassifObjectPtr pObj = find_object(path.back().get(), p, false);
        if (!pObj)
        {
          // if no - try find object by key (in case of k = "area", v = "yes")
          pObj = find_object(path.back().get(), p, true);
        }

        // add to path or stop search
        if (pObj)
          path.push_back(pObj);
        else
          break;

      } while (true);

      // assign type
      uint32_t t = ftype::GetEmptyValue();
      for (size_t i = 0; i < path.size(); ++i)
        ftype::PushValue(t, path[i].GetIndex());

      // use features only with drawing rules
      if (feature::IsDrawableAny(t))
        params.AddType(t);

    } while (true);

    static CachedTypes const types;

    if (!params.house.IsEmpty())
    {
      // Delete "entrance" type for house number (use it only with refs).
      // Add "address" type if we have house number but no valid types.
      if (params.PopExactType(types.Get(CachedTypes::ENTRANCE)))
      {
        params.name.Clear();
        // If we have address (house name or number), we should assign valid type.
        // There are a lot of features like this in Czech Republic.
        params.AddType(types.Get(CachedTypes::ADDRESS));
      }
    }

    for (size_t i = 0; i < params.m_Types.size(); ++i)
      if (types.IsHighway(params.m_Types[i]))
      {
        static char const * arr[][2] = {
          { "oneway", "yes" },
          { "oneway", "1" },
          { "oneway", "-1" },
          { "access", "private" },
          { "lit", "yes" },
        };

        do_find_pairs doFind(arr, ARRAY_SIZE(arr));
        for_each_tag(p, bind<bool>(ref(doFind), _1, _2));

        if (doFind.IsExist(3))
          params.AddType(types.Get(CachedTypes::PRIVATE));

        if (doFind.IsExist(4))
          params.AddType(types.Get(CachedTypes::LIT));

        if (doFind.IsExist(0) || doFind.IsExist(1) || doFind.IsExist(2))
        {
          params.AddType(types.Get(CachedTypes::ONEWAY));

          if (doFind.IsExist(2))
            params.m_reverseGeometry = true;
        }

        break;
      }

    params.FinishAddingTypes();
  }

  uint32_t GetBoundaryType2()
  {
    char const * arr[] = { "boundary", "administrative" };
    return classif().GetTypeByPath(vector<string>(arr, arr + 2));
  }

  bool IsValidTypes(FeatureParams const & params)
  {
    // Add final types checks (during parsing and generation process) here.
    return params.IsValid();
  }
}

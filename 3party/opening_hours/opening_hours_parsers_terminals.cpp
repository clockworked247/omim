#include "opening_hours_parsers.hpp"

namespace osmoh
{
namespace parsing
{
dash_::dash_()
{
  add
      ("-")
      /* not standard */
      // (L"–")(L"—")(L"－")(L"~")(L"～")(L"〜")(L"to")(L"às")(L"ás")(L"as")(L"a")(L"ate")(L"bis")
      ;
}

event_::event_()
{
  add
      ("dawn", osmoh::TimeEvent::Event::Sunrise)
      ("sunrise", osmoh::TimeEvent::Event::Sunrise)
      ("sunset", osmoh::TimeEvent::Event::Sunset)
      ("dusk", osmoh::TimeEvent::Event::Sunset)
      ;
}

wdays_::wdays_()
{
  add
      ("su", 1_weekday)("mo", 2_weekday)("tu", 3_weekday)("we", 4_weekday)("th", 5_weekday)("fr", 6_weekday)("sa", 7_weekday) // en
      // (L"mon", 0)(L"tue", 1)(L"wed", 2)(L"thu", 3)(L"fri", 4)(L"sat", 5)(L"sun", 6) // en
      // (L"пн", 0)(L"вт", 1)(L"ср", 2)(L"чт", 3)(L"пт", 4)(L"сб", 5)(L"вс", 6) // ru
      // (L"пн.", 0)(L"вт.", 1)(L"ср.", 2)(L"чт.", 3)(L"пт.", 4)(L"сб.", 5)(L"вс.", 6) // ru
      // (L"lu", 0)(L"ma", 1)(L"me", 2)(L"je", 3)(L"ve", 4)(L"sa", 5)(L"di", 6) // fr
      // (L"lu", 0)(L"ma", 1)(L"me", 2)(L"gi", 3)(L"ve", 4)(L"sa", 5)(L"do", 6) // it
      // (L"lu", 0)(L"ma", 1)(L"mi", 2)(L"ju", 3)(L"vie", 4)(L"sá", 5)(L"do", 6) // sp
      // (L"週一", 0)(L"週二", 1)(L"週三", 2)(L"週四", 3)(L"週五", 4)(L"週六", 5)(L"週日", 6) // ch traditional
      // (L"senin", 0)(L"selasa", 1)(L"rabu", 2)(L"kamis", 3)(L"jum'at", 4)(L"sabtu", 5)(L"minggu", 6) // indonesian

      // (L"wd", 2)

      ;
}

month_::month_()
{
  add
      ("jan", 1_M)("feb", 2_M)("mar", 3_M)("apr",  4_M)("may",  5_M)("jun",  6_M)
      ("jul", 7_M)("aug", 8_M)("sep", 9_M)("oct", 10_M)("nov", 11_M)("dec", 12_M)
      ;
}

hours_::hours_()
{
  add
      ( "0",  0_h)( "1",  1_h)( "2",  2_h)( "3",  3_h)( "4",  4_h)( "5",  5_h)( "6",  6_h)( "7",  7_h)( "8",  8_h)( "9",  9_h) /* not standard */
      ("00",  0_h)("01",  1_h)("02",  2_h)("03",  3_h)("04",  4_h)("05",  5_h)("06",  6_h)("07",  7_h)("08",  8_h)("09",  9_h)
      ("10", 10_h)("11", 11_h)("12", 12_h)("13", 13_h)("14", 14_h)("15", 15_h)("16", 16_h)("17", 17_h)("18", 18_h)("19", 19_h)
      ("20", 20_h)("21", 21_h)("22", 22_h)("23", 23_h)("24", 24_h)
      ;
}

exthours_::exthours_()
{
  add
      ( "0",  0_h)( "1",  1_h)( "2",  2_h)( "3",  3_h)( "4",  4_h)( "5",  5_h)( "6",  6_h)( "7",  7_h)( "8",  8_h)( "9",  9_h) /* not standard */
      ("00",  0_h)("01",  1_h)("02",  2_h)("03",  3_h)("04",  4_h)("05",  5_h)("06",  6_h)("07",  7_h)("08",  8_h)("09",  9_h)
      ("10", 10_h)("11", 11_h)("12", 12_h)("13", 13_h)("14", 14_h)("15", 15_h)("16", 16_h)("17", 17_h)("18", 18_h)("19", 19_h)
      ("20", 20_h)("21", 21_h)("22", 22_h)("23", 23_h)("24", 24_h)("25", 25_h)("26", 26_h)("27", 27_h)("28", 28_h)("29", 29_h)
      ("30", 30_h)("31", 31_h)("32", 32_h)("33", 33_h)("34", 34_h)("35", 35_h)("36", 36_h)("37", 37_h)("38", 38_h)("39", 39_h)
      ("40", 40_h)("41", 41_h)("42", 42_h)("43", 43_h)("44", 44_h)("45", 45_h)("46", 46_h)("47", 47_h)("48", 48_h)
      ;
}

minutes_::minutes_()
{
  add
      ( "0",  0_min)( "1",  1_min)( "2",  2_min)( "3",  3_min)( "4",  4_min)( "5",  5_min)( "6",  6_min)( "7",  7_min)( "8",  8_min)( "9",  9_min) /* not standard */
      ("00",  0_min)("01",  1_min)("02",  2_min)("03",  3_min)("04",  4_min)("05",  5_min)("06",  6_min)("07",  7_min)("08",  8_min)("09",  9_min)
      ("10", 10_min)("11", 11_min)("12", 12_min)("13", 13_min)("14", 14_min)("15", 15_min)("16", 16_min)("17", 17_min)("18", 18_min)("19", 19_min)
      ("20", 20_min)("21", 21_min)("22", 22_min)("23", 23_min)("24", 24_min)("25", 25_min)("26", 26_min)("27", 27_min)("28", 28_min)("29", 29_min)
      ("30", 30_min)("31", 31_min)("32", 32_min)("33", 33_min)("34", 34_min)("35", 35_min)("36", 36_min)("37", 37_min)("38", 38_min)("39", 39_min)
      ("40", 40_min)("41", 41_min)("42", 42_min)("43", 43_min)("44", 44_min)("45", 45_min)("46", 46_min)("47", 47_min)("48", 48_min)("49", 49_min)
      ("50", 50_min)("51", 51_min)("52", 52_min)("53", 53_min)("54", 54_min)("55", 55_min)("56", 56_min)("57", 57_min)("58", 58_min)("59", 59_min)
      ;
}

weeknum_::weeknum_()
{
  add
      ( "1",  1)( "2",  2)( "3",  3)( "4",  4)( "5",  5)( "6",  6)( "7",  7)( "8",  8)( "9",  9)
      ("01",  1)("02",  2)("03",  3)("04",  4)("05",  5)("06",  6)("07",  7)("08",  8)("09",  9)
      ("10", 10)("11", 11)("12", 12)("13", 13)("14", 14)("15", 15)("16", 16)("17", 17)("18", 18)("19", 19)
      ("20", 20)("21", 21)("22", 22)("23", 23)("24", 24)("25", 25)("26", 26)("27", 27)("28", 28)("29", 29)
      ("30", 30)("31", 31)("32", 32)("33", 33)("34", 34)("35", 35)("36", 36)("37", 37)("38", 38)("39", 39)
      ("40", 40)("41", 41)("42", 42)("43", 43)("44", 44)("45", 45)("46", 46)("47", 47)("48", 48)("49", 49)
      ("50", 50)("51", 51)("52", 52)("53", 53)
      ;
}

daynum_::daynum_()
{
  add
      ("1",  1)("2",  2)("3",  3)("4",  4)("5",  5)("6",  6)("7",  7)("8",  8)("9",  9)
      ("01",  1)("02",  2)("03",  3)("04",  4)("05",  5)("06",  6)("07",  7)("08",  8)("09",  9)
      ("10", 10)("11", 11)("12", 12)("13", 13)("14", 14)("15", 15)("16", 16)("17", 17)("18", 18)("19", 19)
      ("20", 20)("21", 21)("22", 22)("23", 23)("24", 24)("25", 25)("26", 26)("27", 27)("28", 28)("29", 29)
      ("30", 30)("31", 31)
      ;
}
} // namespace parsing
} // namespace osmoh

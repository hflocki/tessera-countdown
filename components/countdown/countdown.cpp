#include "countdown.h"
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <string>

namespace esphome::countdown {

using tessera::Font;
namespace ui = tessera::ui;

// ---- Days and dates -----------------------------------------------------------------------------------------------

// Days since 1970-01-01 for a date (Howard Hinnant's days_from_civil): the difference of two of these is whole days.
static int32_t civil(int y, int m, int d) {
  y -= m <= 2;
  const int era = (y >= 0 ? y : y - 399) / 400;
  const unsigned yoe = static_cast<unsigned>(y - era * 400);
  const unsigned doy = (153 * (m + (m > 2 ? -3 : 9)) + 2) / 5 + d - 1;
  return era * 146097 + static_cast<int32_t>(yoe * 365 + yoe / 4 - yoe / 100 + doy) - 719468;
}

// The month and day of a day number (Howard Hinnant's civil_from_days).
static void from_civil(int32_t z, int &y, int &m, int &d) {
  z += 719468;
  const int era = (z >= 0 ? z : z - 146096) / 146097;
  const unsigned doe = static_cast<unsigned>(z - era * 146097);
  const unsigned yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
  const unsigned doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
  const unsigned mp = (5 * doy + 2) / 153;
  d = static_cast<int>(doy - (153 * mp + 2) / 5 + 1);
  m = static_cast<int>(mp < 10 ? mp + 3 : mp - 9);
  y = static_cast<int>(yoe) + era * 400 + (m <= 2);
}

// The weekday of a day number, 0 Sunday to 6 Saturday: 1 January 1970 was a Thursday.
static int weekday(int32_t day) { return ((day + 4) % 7 + 7) % 7; }

// Easter Sunday of a year (the anonymous Gregorian algorithm), as a day number.
static int32_t easter(int y) {
  const int a = y % 19, b = y / 100, c = y % 100, d = b / 4, e = b % 4, f = (b + 8) / 25, g = (b - f + 1) / 3;
  const int h = (19 * a + b - d - g + 15) % 30, i = c / 4, k = c % 4, l = (32 + 2 * e + 2 * i - h - k) % 7;
  const int m = (a + 11 * h + 22 * l) / 451, month = (h + l - 7 * m + 114) / 31, day = (h + l - 7 * m + 114) % 31 + 1;
  return civil(y, month, day);
}

// The first Sunday of Advent: four Sundays before Christmas, the fourth the Sunday on or before 24 December.
static int32_t advent(int y) {
  const int32_t eve = civil(y, 12, 24);
  return eve - weekday(eve) - 21;
}

// The next day of a yearly date from `today` on: this year's, or next year's once it has passed.
static int32_t next_yearly(int32_t today, int year, int month, int day) {
  const int32_t d = civil(year, month, day);
  return d >= today ? d : civil(year + 1, month, day);
}
// How many days from `from` up to before `to` fall on one of the weekdays in `mask` (bit 0 Sunday).
static int32_t count_weekdays(int32_t from, int32_t to, uint8_t mask) {
  if (to <= from) return 0;
  int per_week = 0;
  for (int i = 0; i < 7; ++i) per_week += (mask >> i) & 1;
  const int32_t span = to - from, weeks = span / 7;
  int32_t n = weeks * per_week;
  for (int32_t d = from + weeks * 7; d < to; ++d) n += (mask >> weekday(d)) & 1;
  return n;
}

static int32_t next_of(int32_t today, int year, int32_t (*of)(int)) {
  const int32_t d = of(year);
  return d >= today ? d : of(year + 1);
}

// A date a person typed: "24.12." or "12-24" every year, "2027-05-01" or "1.5.2027" once. False for anything else.
static bool parse_date(const char *text, int &y, int &m, int &d) {
  y = 0;
  int a = 0, b = 0, c = 0;
  if (sscanf(text, "%d-%d-%d", &a, &b, &c) == 3 && a > 31) y = a, m = b, d = c;
  else if (sscanf(text, "%d.%d.%d", &a, &b, &c) == 3 && c > 31) d = a, m = b, y = c;
  else if (sscanf(text, "%d.%d", &a, &b) == 2) d = a, m = b;
  else if (sscanf(text, "%d-%d", &a, &b) == 2) m = a, d = b;
  else return false;
  return m >= 1 && m <= 12 && d >= 1 && d <= 31;
}

// A moment as Home Assistant writes a state: "2026-12-24", "2026-12-24 18:00:00" (local, an input_datetime) or
// "2026-12-24T17:00:00+00:00" (a timestamp sensor). Gives the local day, and the moment when it has a time.
static bool parse_state(const std::string &s, int32_t &day, uint32_t &at) {
  int y, mo, d, h = -1, mi = 0, sec = 0;
  if (sscanf(s.c_str(), "%4d-%2d-%2d", &y, &mo, &d) != 3 || mo < 1 || mo > 12 || d < 1 || d > 31) return false;
  at = 0;
  if (s.size() >= 16) sscanf(s.c_str() + 11, "%2d:%2d:%2d", &h, &mi, &sec);
  const size_t zone = s.size() > 19 ? s.find_first_of("+-Z", 19) : std::string::npos;
  if (h >= 0 && zone != std::string::npos) {
    // A moment with its zone: its local day is the screen's.
    long offset = 0;
    if (s[zone] != 'Z') {
      int oh = 0, om = 0;
      sscanf(s.c_str() + zone + 1, "%2d:%2d", &oh, &om);
      offset = (oh * 60L + om) * 60L * (s[zone] == '-' ? -1 : 1);
    }
    const long long t = static_cast<long long>(civil(y, mo, d)) * 86400 + h * 3600 + mi * 60 + sec - offset;
    if (t <= 0) return false;
    at = static_cast<uint32_t>(t);
    const tessera::LocalTime lt = tessera::local_time(at);
    day = civil(lt.year, lt.month, lt.day);
    return true;
  }
  day = civil(y, mo, d);
  if (h > 0 || mi > 0) at = 1;  // a local time of day: shown as such on the day, never compared
  return true;
}

// ---- What to count down to ----------------------------------------------------------------------------------------

enum class Kind : uint8_t { CHRISTMAS, NEW_YEARS_EVE, EASTER, ADVENT, HALLOWEEN, SUMMER, DATE, ENTITY, NONE };

// The kinds a person can choose, by their value in the manifest.
static Kind kind_of(const char *v) {
  if (strcmp(v, "christmas") == 0) return Kind::CHRISTMAS;
  if (strcmp(v, "new_years_eve") == 0) return Kind::NEW_YEARS_EVE;
  if (strcmp(v, "easter") == 0) return Kind::EASTER;
  if (strcmp(v, "advent") == 0) return Kind::ADVENT;
  if (strcmp(v, "halloween") == 0) return Kind::HALLOWEEN;
  if (strcmp(v, "summer") == 0) return Kind::SUMMER;
  if (strcmp(v, "date") == 0) return Kind::DATE;
  if (strcmp(v, "entity") == 0) return Kind::ENTITY;
  return Kind::CHRISTMAS;
}

// The text key of a holiday: "christmas" for "until Christmas", "christmas_today" on the day.
static const char *key_of(Kind k) {
  switch (k) {
    case Kind::CHRISTMAS: return "christmas";
    case Kind::NEW_YEARS_EVE: return "new_years_eve";
    case Kind::EASTER: return "easter";
    case Kind::ADVENT: return "advent";
    case Kind::HALLOWEEN: return "halloween";
    case Kind::SUMMER: return "summer";
    default: return nullptr;
  }
}

// The day of a holiday from `today` on.
static int32_t holiday(Kind k, int32_t today, int year) {
  switch (k) {
    case Kind::CHRISTMAS: return next_yearly(today, year, 12, 24);
    case Kind::NEW_YEARS_EVE: return next_yearly(today, year, 12, 31);
    case Kind::EASTER: return next_of(today, year, easter);
    case Kind::ADVENT: return next_of(today, year, advent);
    case Kind::HALLOWEEN: return next_yearly(today, year, 10, 31);
    case Kind::SUMMER: return next_yearly(today, year, 6, 21);
    default: return today;
  }
}

// The picture of a holiday, or of the season of a day.
static const char *motif_of(Kind k, int32_t day) {
  switch (k) {
    case Kind::CHRISTMAS: return "christmas";
    case Kind::NEW_YEARS_EVE: return "new_year";
    case Kind::EASTER: return "easter";
    case Kind::ADVENT: return "advent";
    case Kind::HALLOWEEN: return "halloween";
    case Kind::SUMMER: return "summer";
    default: break;
  }
  int y, m, d;
  from_civil(day, y, m, d);
  if (m == 12 && d >= 20 && d <= 26) return "christmas";
  if ((m == 12 && d >= 30) || (m == 1 && d == 1)) return "new_year";
  if (m == 10 && d == 31) return "halloween";
  if (m == 12 || m <= 2) return "winter";
  if (m <= 5) return "spring";
  if (m <= 8) return "summer";
  return "autumn";
}

static std::string motif_url(const char *motif) {
  return std::string("https://raw.githubusercontent.com/hflocki/tessera-countdown/main/motifs/") + motif + ".png";
}

// ---- The tile -----------------------------------------------------------------------------------------------------

// How the picture and the number share the tile (the option `layout`).
enum class Layout : uint8_t {
  SIDE,        // the number at the left, the picture over the rest, to the tile's edges
  BACKGROUND,  // the picture over the whole tile, the number on a dark veil at its left
  BANNER,      // the picture as a band over the tile's top, the number and its words in a row under it
  PLAIN,       // no picture: the number and its words in the middle
};

// A picture of the season and the number of days until a day, with what they count to. On the day itself the
// holiday's wish ("Merry Christmas") instead of the number, or the time of a calendar's event.
class CountdownTile : public tessera::Tile {
 public:
  explicit CountdownTile(const tessera::Plugin *plugin) : plugin_(plugin) {}

  void create(const tessera::TileContext &c) override {
    width_ = c.width;
    height_ = c.height;
    const char *event = c.options["event"] | "christmas";
    kind_ = kind_of(event);
    // Automatic: the nearest of the three holidays.
    auto_ = c.options["auto"] | true;
    has_date_ = parse_date(c.options["date"] | "", year_, month_, day_);
    snprintf(what_, sizeof what_, "%s", c.options["what"] | "");
    snprintf(entity_, sizeof entity_, "%s", c.options["entity"] | "");
    snprintf(motif_, sizeof motif_, "%s", c.options["motif"] | "auto");
    const char *unit = c.options["unit"] | "days";
    unit_ = strcmp(unit, "weeks") == 0 ? 'w' : strcmp(unit, "workdays") == 0 ? 'a' : strcmp(unit, "fridays") == 0 ? 'f' : 'd';
    const char *layout = c.options["layout"] | "background";

    // The layout: a tile of one column has no room for the number beside the picture.
    layout_ = strcmp(layout, "side") == 0 ? Layout::SIDE : strcmp(layout, "banner") == 0 ? Layout::BANNER
                                                                                          : Layout::BACKGROUND;
    if (layout_ == Layout::SIDE && c.columns < 2) layout_ = c.rows >= 2 ? Layout::BANNER : Layout::BACKGROUND;
    if (!ui::pictures() || strcmp(motif_, "none") == 0) layout_ = Layout::PLAIN;
    gap_ = ui::px(ui::large() ? 14 : 10);

    // The picture reaches the tile's own edges, over its padding, with the tile's corners.
    const int pl = lv_obj_get_style_pad_left(c.parent, LV_PART_MAIN), pt = lv_obj_get_style_pad_top(c.parent, LV_PART_MAIN);
    const int pr = lv_obj_get_style_pad_right(c.parent, LV_PART_MAIN);
    const int pb = lv_obj_get_style_pad_bottom(c.parent, LV_PART_MAIN);
    const int radius = lv_obj_get_style_radius(c.parent, LV_PART_MAIN);
    x0_ = 0, y0_ = 0, w_ = width_, h_ = height_;
    switch (layout_) {
      case Layout::SIDE: {
        w_ = width_ * 42 / 100;
        const int x = w_ + gap_;
        place_ = make_place(c.parent, x, -pt, width_ - x + pr, height_ + pt + pb, radius);
        break;
      }
      case Layout::BACKGROUND: {
        place_ = make_place(c.parent, -pl, -pt, width_ + pl + pr, height_ + pt + pb, radius);
        // A veil of the instrument's dark ground, strong at the left and gone at the right, so the number reads on
        // any picture. On a tile of one column it covers evenly.
        veil_ = ui::block(c.parent, theme::INSTRUMENT);
        lv_obj_remove_flag(veil_, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_set_style_radius(veil_, radius, 0);
        lv_obj_set_size(veil_, width_ + pl + pr, height_ + pt + pb);
        lv_obj_set_pos(veil_, -pl, -pt);
        if (c.columns >= 2) {
          lv_obj_set_style_bg_grad_color(veil_, ui::color(theme::INSTRUMENT), 0);
          lv_obj_set_style_bg_grad_dir(veil_, LV_GRAD_DIR_HOR, 0);
          lv_obj_set_style_bg_main_opa(veil_, LV_OPA_80, 0);
          lv_obj_set_style_bg_grad_opa(veil_, LV_OPA_TRANSP, 0);
          lv_obj_set_style_bg_main_stop(veil_, 70, 0);
          lv_obj_set_style_bg_grad_stop(veil_, 200, 0);
          w_ = width_ * 55 / 100;
        } else {
          lv_obj_set_style_bg_opa(veil_, LV_OPA_50, 0);
        }
        break;
      }
      case Layout::BANNER: {
        // The row under the band holds the largest number that fits a third of the tile, at least a title's line.
        strip_ = std::max(ui::line_height(Font::TITLE), std::min(height_ * 2 / 5, ui::line_height(Font::DIGITS)));
        y0_ = height_ - strip_;
        h_ = strip_;
        place_ = make_place(c.parent, -pl, -pt, width_ + pl + pr, pt + y0_ - gap_ / 2, radius);
        break;
      }
      case Layout::PLAIN:
        break;
    }

    number_ = ui::label(c.parent, Font::DIGITS, theme::INK);
    words_ = ui::text(c.parent, Font::BODY, theme::INK_SOFT);
    date_ = ui::label(c.parent, Font::BODY, theme::MUTED);
    const lv_text_align_t align = layout_ == Layout::PLAIN ? LV_TEXT_ALIGN_CENTER : LV_TEXT_ALIGN_LEFT;
    for (lv_obj_t *o : {number_, words_, date_}) lv_obj_set_style_text_align(o, align, 0);
    on_theme();
  }

  void on_state(JsonObjectConst data) override {
    // The chosen calendar or entity: a calendar's next event (start_time, message), else the state as a date.
    has_entity_ = false;
    JsonObjectConst e = data["entities"]["entity"];
    if (kind_ != Kind::ENTITY || e.isNull()) return;
    const std::string state = e["state"] | "";
    snprintf(entity_name_, sizeof entity_name_, "%s", e["name"] | "");
    entity_at_ = 0;
    if (strncmp(entity_, "calendar.", 9) == 0) {
      const uint32_t start = e["attributes"]["start_time"] | 0u;
      const char *message = e["attributes"]["message"] | "";
      if (start) {
        const tessera::LocalTime lt = tessera::local_time(start);
        entity_day_ = civil(lt.year, lt.month, lt.day);
        entity_at_ = lt.hour || lt.minute ? start : 0;
        if (*message) snprintf(entity_name_, sizeof entity_name_, "%s", message);
        has_entity_ = true;
      }
    } else {
      has_entity_ = parse_state(state, entity_day_, entity_at_);
    }
  }

  void on_tick(uint32_t now) override {
    if (!now) return;  // the screen's clock is not set yet
    const tessera::LocalTime lt = tessera::local_time(now);
    const int32_t today = civil(lt.year, lt.month, lt.day);

    // What it counts to now: its day, its kind and, for a calendar's event, its moment.
    Kind kind = kind_;
    int32_t day = today;
    uint32_t at = 0;
    bool known = true;
    if (auto_) {
      day = INT32_MAX;
      for (Kind k : {Kind::CHRISTMAS, Kind::NEW_YEARS_EVE, Kind::EASTER}) {
        const int32_t d = holiday(k, today, lt.year);
        if (d < day) day = d, kind = k;
      }
    } else if (kind_ == Kind::DATE) {
      known = has_date_;
      if (known) day = year_ ? civil(year_, month_, day_) : next_yearly(today, lt.year, month_, day_);
    } else if (kind_ == Kind::ENTITY) {
      known = has_entity_;
      day = entity_day_;
      at = entity_at_ > 1 ? entity_at_ : 0;
    } else {
      day = holiday(kind_, today, lt.year);
    }
    const int32_t left = day - today;

    std::string number, words, date;
    if (!known) {
      words = plugin_->text(kind_ == Kind::DATE ? "no_date" : "no_entity");
    } else {
      const char *key = key_of(kind);
      const std::string name = *what_ ? std::string(what_) : kind == Kind::ENTITY && *entity_name_ ? entity_name_ : "";
      // "until Christmas", "until Holidays"; on the day the holiday's wish or the name.
      std::string until = key && !*what_ ? plugin_->text(key)
                          : tessera::fill(plugin_->text("until"), "what", name.empty() ? plugin_->text("event") : name);
      if (left > 0) {
        // In the unit chosen: days, weeks and the days over, working days (Monday to Friday, today among them while
        // it is one) or the Fridays between today and the day.
        int32_t n = left;
        std::string unit = tessera::format(plugin_->text("days"), left);
        if (unit_ == 'w' && left >= 7) {
          n = left / 7;
          unit = tessera::format(plugin_->text("weeks"), n);
          if (left % 7) unit += " " + tessera::format(plugin_->text("and_days"), left % 7);
        } else if (unit_ == 'a') {
          n = count_weekdays(today, day, 0x3E);
          unit = tessera::format(plugin_->text("workdays"), n);
        } else if (unit_ == 'f') {
          n = count_weekdays(today + 1, day, 0x20);
          unit = tessera::format(plugin_->text("fridays"), n);
        }
        number = std::to_string(n);
        words = unit + " " + until;
        // Noon of that day, so a change of summer time never moves it to another date.
        const uint32_t noon = now - (lt.hour * 3600 + lt.minute * 60 + lt.second) + 12 * 3600 + left * 86400;
        date = tessera::date_text(noon);
      } else if (left == 0) {
        words = key && !*what_ ? plugin_->text((std::string(key) + "_today").c_str())
                : name.empty() ? std::string(plugin_->text("today")) : name;
        date = at ? tessera::fill(plugin_->text("at"), "time", tessera::clock_text(at)) : plugin_->text("today");
      } else {
        words = name.empty() ? std::string(plugin_->text("event")) : name;
        date = plugin_->text("passed");
      }
    }
    if (layout_ == Layout::BANNER) paint_row(number, words);
    else paint_column(number, words, date);

    if (place_) {
      const char *m = strcmp(motif_, "auto") == 0 ? motif_of(kind, day) : motif_;
      ui::set_picture(place_, motif_url(m), tessera::Fit::COVER);
    }
  }

  void on_theme() override {
    // On the veil the instrument's ink, which stays light in both looks; elsewhere the screen's own.
    const bool veiled = layout_ == Layout::BACKGROUND;
    ui::set_color(number_, veiled ? theme::INSTRUMENT_INK : theme::INK);
    ui::set_color(words_, veiled ? theme::INSTRUMENT_INK : theme::INK_SOFT);
    ui::set_color(date_, veiled ? theme::INSTRUMENT_MUTED : theme::MUTED);
    on_tick(tessera::epoch());
  }

 private:
  static lv_obj_t *make_place(lv_obj_t *parent, int x, int y, int w, int h, int radius) {
    lv_obj_t *place = ui::picture(parent);
    lv_obj_set_size(place, w, h);
    lv_obj_set_pos(place, x, y);
    lv_obj_set_style_radius(place, radius, 0);
    return place;
  }

  // The largest face of the number that fits `w` wide and `h` high.
  static Font number_face(const std::string &number, int w, int h) {
    for (Font f : {Font::DIGITS, Font::VALUE, Font::HEADLINE}) {
      if (f == Font::DIGITS && !ui::covers(number, f)) continue;
      if (ui::line_height(f) <= h && ui::text_width(number, f) <= w) return f;
    }
    return Font::TITLE;
  }

  // The number above its words and the date, in the room for words (x0_, y0_, w_, h_), together in its middle.
  void paint_column(const std::string &number, const std::string &words, const std::string &date) {
    const int body_h = ui::line_height(Font::BODY);
    const Font face = number.empty() ? Font::TITLE : number_face(number, w_, h_ - body_h);
    const int nh = number.empty() ? 0 : ui::line_height(face);
    // On the day the words are the headline.
    const Font words_font = number.empty() ? Font::TITLE : Font::BODY;
    const int line = ui::line_height(words_font);
    int wh = (ui::text_width(words, words_font) > w_ ? 2 : 1) * line;
    if (nh + wh > h_) wh = line;
    const int dh = !date.empty() && nh + wh + body_h <= h_ ? body_h : 0;
    const int top = y0_ + std::max(0, (h_ - nh - wh - dh) / 2);
    put(number_, number, face, x0_, top, w_, nh);
    put(words_, words, words_font, x0_, top + nh, w_, wh);
    put(date_, dh ? date : "", Font::BODY, x0_, top + nh + wh, w_, body_h);
  }

  // The number and, right of it, its words, in the row under the band.
  void paint_row(const std::string &number, const std::string &words) {
    const Font face = number.empty() ? Font::TITLE : number_face(number, w_ / 2, h_);
    const int nh = number.empty() ? 0 : ui::line_height(face);
    const int nw = number.empty() ? 0 : ui::text_width(number, face) + ui::px(2);
    const int x = nw ? nw + gap_ / 2 : 0, w = std::max(1, w_ - x);
    const Font words_font = number.empty() ? Font::TITLE : Font::BODY;
    const int line = ui::line_height(words_font);
    const int wh = ui::text_width(words, words_font) > w && 2 * line <= h_ ? 2 * line : line;
    put(number_, number, face, 0, y0_ + (h_ - nh) / 2, nw, nh);
    put(words_, words, words_font, x, y0_ + (h_ - wh) / 2, w, wh);
    put(date_, "", Font::BODY, 0, y0_, 1, 1);
  }

  static void put(lv_obj_t *o, const std::string &text, Font font, int x, int y, int w, int h) {
    ui::set_font(o, font);
    ui::set_text(o, text);
    lv_obj_set_size(o, std::max(1, w), std::max(1, h));
    lv_obj_set_pos(o, x, y);
  }

  const tessera::Plugin *plugin_;
  Layout layout_ = Layout::BACKGROUND;
  int width_ = 0, height_ = 0, x0_ = 0, y0_ = 0, w_ = 0, h_ = 0, gap_ = 0, strip_ = 0;
  Kind kind_ = Kind::CHRISTMAS;
  bool auto_ = true, has_date_ = false, has_entity_ = false;
  int year_ = 0, month_ = 0, day_ = 0;
  int32_t entity_day_ = 0;
  uint32_t entity_at_ = 0;
  char unit_ = 'd';  // d days, w weeks, a working days, f Fridays
  char what_[65]{}, entity_[96]{}, entity_name_[65]{}, motif_[16]{};
  lv_obj_t *place_{}, *veil_{}, *number_{}, *words_{}, *date_{};
};

void Countdown::setup() {
  add_tile("countdown", [this]() { return new CountdownTile(this); });
}

}  // namespace esphome::countdown

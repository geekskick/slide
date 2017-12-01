#include "slide.h"
#include <iostream>

std::pair<int, int> slide::RenderScale(Page &p, Slide &slide,
                                       const Color &color, int xoff, int yoff,
                                       float scale) {
  std::cerr << "render_scale" << std::endl;
  int w = 0;
  int h = 0;
  int line_height = 0;
  int line_width = 0;
  int prev_line = -1;
  for (auto token : slide) {
    if (token.line_ != prev_line) {
      h = h + line_height;
      line_width = 0;
      prev_line = token.line_;
    }
    line_height = p.TextHeight(token.style_, scale);
    int token_width = p.TextWidth(token.text_, token.style_, scale);
    int x = xoff + line_width;
    int y = yoff + h;
    if (color != 0) {
      p.Text(token.text_, color, x, y, token.style_, scale);
    }
    line_width = line_width + token_width;
    if (line_width > w) {
      w = line_width;
    }
  }
  h = h + line_height;
  return std::make_pair(xoff + w, yoff + h);
}

void slide::Render(Page &page, Slide &slide, const Color &fg, const Color &bg) {
  if (slide.size() == 0) {
    return;
  }
  std::cerr << "render" << std::endl;
  float scale = 1.f;
  auto size = RenderScale(page, slide, 0, 0, 0, scale);
  scale = std::min(page.Size().Width() * 0.8 / size.first,
                   page.Size().Height() * 0.8 / size.second);
  size = RenderScale(page, slide, 0, 0, 0, scale);

  page.Background(bg);
  RenderScale(page, slide, fg, ( page.Size().Width() - size.first) / 2,
              ( page.Size().Height() - size.second) / 2, scale);

  std::cerr << "render complete" << std::endl;
}

slide::Deck slide::Parse(const std::string &text) {
  Deck deck{};
  auto lineno = 0;
  auto begin = text.cbegin();
  for (auto it = begin, end = text.cend(); it < end;) {
    // Skip leading newlines
    for (; *it == '\n' && it < end; ++it, ++lineno)
      ;
    // Parse next slide
    Slide slide{};
    for (; *it != '\n' && it < end; ++it, ++lineno) {
      std::string text;
      Style style{Style::Normal};
      switch (*it) {
      case '@':
        ++it;
        // TODO: parse image path and geometry
        for (; it < end && *it != '\n'; ++it) {
        }
        break;
      case '#':
        ++it;
        style = Style::Header;
        for (; it < end && *it == ' '; ++it)
          ;
        break;
      case ' ':
        ++it;
        if (it < end && *it == ' ') {
          ++it;
          style = Style::Monospace;
        } else {
          text.push_back(' ');
        }
        break;
      case '.':
        ++it;
        break;
      }
      bool insert_empty_token = true;
      for (; it < end && *it != '\n'; ++it) {
        if (style == Style::Normal && *it == '*') {
          ++it;
          if (it < end && *it != ' ' && *it != '*') {
            std::string em;
            for (; it < end && *it != '*' && *it != '\n'; ++it) {
              em.push_back(*it);
            }
            if (*it == '*') {
              if (text.size() > 0) {
                slide.push_back(
                    Token{std::distance(begin, it), lineno, style, text});
              }
              slide.push_back(
                  Token{std::distance(begin, it), lineno, Style::Strong, em});
              insert_empty_token = false;
              text = "";
            } else {
              text.push_back('*');
              text.append(em);
            }
          } else {
            text.push_back('*');
            if (*it != '*') {
              text.push_back(*it);
            }
          }
        } else {
          text.push_back(*it);
        }
      }
      if (insert_empty_token || text.size() > 0) {
        slide.push_back(Token{std::distance(begin, it), lineno, style, text});
      }
    }
    // Skip trailing newlines
    for (; *it == '\n' && it < end; ++it, ++lineno)
      ;
    if (slide.size() > 0) {
      deck.push_back(slide);
    }
  }
  return deck;
}
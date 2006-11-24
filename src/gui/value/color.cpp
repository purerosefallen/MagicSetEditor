//+----------------------------------------------------------------------------+
//| Description:  Magic Set Editor - Program to make Magic (tm) cards          |
//| Copyright:    (C) 2001 - 2006 Twan van Laarhoven                           |
//| License:      GNU General Public License 2 or later (see file COPYING)     |
//+----------------------------------------------------------------------------+

// ----------------------------------------------------------------------------- : Includes

#include <gui/value/color.hpp>
#include <gui/drop_down_list.hpp>
#include <gui/util.hpp>
#include <data/action/value.hpp>
#include <wx/colordlg.h>

DECLARE_TYPEOF_COLLECTION(ColorField::ChoiceP);

// ----------------------------------------------------------------------------- : DropDownColorList

// A drop down list of color choices
class DropDownColorList : public DropDownList {
  public:
	DropDownColorList(Window* parent, ColorValueEditor& cve);
	
  protected:		
	virtual size_t itemCount() const;
	virtual bool   lineBelow(size_t item) const;
	virtual String itemText(size_t item) const;
	virtual void   drawIcon(DC& dc, int x, int y, size_t item, bool selected) const;
	
	virtual void   select(size_t item);
	virtual size_t selection() const;
	
  private:
	ColorValueEditor& cve;
	mutable Color default_color;

	inline const ColorField& field() const { return cve.field(); }
//	// default, custom item
	bool hasDefault() const { return field().default_script; }
	bool hasCustom()  const { return field().allow_custom; }
};


DropDownColorList::DropDownColorList(Window* parent, ColorValueEditor& cve)
	: DropDownList(parent, false, &cve)
	, cve(cve)
{
	icon_size.width = 25;
	if (item_size.height < 16) {
		text_offset = (16 - item_size.height) / 2;
		item_size.height = 16;
	}
}

size_t DropDownColorList::itemCount() const {
	return cve.field().choices.size() + hasDefault() + hasCustom();
}
bool DropDownColorList::lineBelow(size_t item) const {
	return (item == 0               && hasDefault()) // below default item
		|| (item == itemCount() - 2 && hasCustom()); // above custom item
}
String DropDownColorList::itemText(size_t item) const {
	if (item == 0 && hasDefault()) {
		return field().default_name;
	} else if (item == itemCount()-1 && hasCustom()) {
		return _("Custom...");
	} else {
		return field().choices[item - hasDefault()]->name;
	}
}

void DropDownColorList::drawIcon(DC& dc, int x, int y, size_t item, bool selected) const {
	Color col;
	if (item == 0 && hasDefault()) { // default
		col = default_color;
	} else if (item == itemCount()-1 && hasCustom()) { // custom color
		col = cve.value().value();
	} else {
		col = field().choices[item - hasDefault()]->color;
	}
	// draw a rectangle with the right color
	dc.SetPen(wxSystemSettings::GetColour(selected ? wxSYS_COLOUR_HIGHLIGHTTEXT : wxSYS_COLOUR_WINDOWTEXT));
	dc.SetBrush(col);
	dc.DrawRectangle(x+1, y+1, icon_size.width-2, item_size.height-2);
}

size_t DropDownColorList::selection() const {
	// find selected color
	size_t selection = hasCustom() ? itemCount() - 1 : NO_SELECTION;
	size_t i = 0;
	FOR_EACH_CONST(c, field().choices) {
		if (c->color == cve.value().value()) {
			selection = i + hasDefault();
			break;
		}
		i++;
	}
	// has default item?
	if (hasDefault() && cve.value().value.isDefault()) {
		// default is selected
		default_color = cve.value().value();
		return 0;
	} else if (hasDefault()) {
		// evaluate script to find default color
		default_color = *field().default_script.invoke(cve.viewer.getContext());
	}
	return selection;
}
void DropDownColorList::select(size_t item) {
	if (item == 0 && hasDefault()) {
		cve.change( Defaultable<Color>());
	} else if (item == itemCount() - 1 && hasCustom()) {
		cve.changeCustom();
	} else {
		cve.change(field().choices[item - hasDefault()]->color);
	}
}

// ----------------------------------------------------------------------------- : ColorValueEditor

IMPLEMENT_VALUE_EDITOR(Color)
	, drop_down(new DropDownColorList(&editor(), *this))
{}

void ColorValueEditor::onLeftDown(const RealPoint& pos, wxMouseEvent& ev) {
	drop_down->onMouseInParent(ev, !nativeLook());
}
void ColorValueEditor::onChar(wxKeyEvent& ev) {
	drop_down->onCharInParent(ev);
}
void ColorValueEditor::onLoseFocus() {
	drop_down->hide(false);
}

void ColorValueEditor::drawSelection(RotatedDC& dc) {
	if (nativeLook()) {
		draw_drop_down_arrow(&editor(), dc.getDC(), style().getRect().grow(1), drop_down->IsShown());
	}
}
void ColorValueEditor::determineSize() {
	style().height = 20;
}

void ColorValueEditor::change(const Defaultable<Color>& c) {
	getSet().actions.add(value_action(static_pointer_cast<ColorValue>(valueP), c));
}
void ColorValueEditor::changeCustom() {
	Color c = wxGetColourFromUser(0, value().value());
	if (c.Ok()) change(c);
}
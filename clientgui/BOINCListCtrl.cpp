// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2008 University of California
//
// BOINC is free software; you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.
//
// BOINC is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with BOINC.  If not, see <http://www.gnu.org/licenses/>.

#if defined(__GNUG__) && !defined(__APPLE__)
#pragma implementation "BOINCListCtrl.h"
#endif

#include "stdwx.h"
#include "BOINCBaseView.h"
#include "BOINCListCtrl.h"
#include "Events.h"


DEFINE_EVENT_TYPE(wxEVT_CHECK_SELECTION_CHANGED)

#if USE_NATIVE_LISTCONTROL
DEFINE_EVENT_TYPE(wxEVT_DRAW_PROGRESSBAR)
#endif

BEGIN_EVENT_TABLE(CBOINCListCtrl, LISTCTRL_BASE)

#if USE_NATIVE_LISTCONTROL
    EVT_DRAW_PROGRESSBAR(CBOINCListCtrl::OnDrawProgressBar)
#else
#ifdef __WXMAC__
	EVT_SIZE(CBOINCListCtrl::OnSize)    // In MacAccessibility.mm
#endif
#endif

#if ! USE_LIST_CACHE_HINT
    EVT_LEFT_DOWN(CBOINCListCtrl::OnMouseDown)
#endif
END_EVENT_TABLE()


BEGIN_EVENT_TABLE(MyEvtHandler, wxEvtHandler)
    EVT_PAINT(MyEvtHandler::OnPaint)
END_EVENT_TABLE()


IMPLEMENT_DYNAMIC_CLASS(CBOINCListCtrl, LISTCTRL_BASE)


CBOINCListCtrl::CBOINCListCtrl() {}


CBOINCListCtrl::CBOINCListCtrl(
    CBOINCBaseView* pView, wxWindowID iListWindowID, wxInt32 iListWindowFlags
) : LISTCTRL_BASE(
    pView, iListWindowID, wxDefaultPosition, wxSize(-1, -1), iListWindowFlags
) {
    m_pParentView = pView;

    // Enable Zebra Striping
    EnableAlternateRowColours(true);

#if USE_NATIVE_LISTCONTROL
    m_bProgressBarEventPending = false;
#else
#ifdef __WXMAC__
    SetupMacAccessibilitySupport();
#endif
#endif
}


CBOINCListCtrl::~CBOINCListCtrl()
{
    m_iRowsNeedingProgressBars.Clear();
#ifdef __WXMAC__
#if !USE_NATIVE_LISTCONTROL
    RemoveMacAccessibilitySupport();
#endif
#endif
}


bool CBOINCListCtrl::OnSaveState(wxConfigBase* pConfig) {
    wxString    strBaseConfigLocation = wxEmptyString;
    wxListItem  liColumnInfo;
    wxInt32     iIndex = 0;
    wxInt32     iColumnCount = 0;


    wxASSERT(pConfig);


    // Retrieve the base location to store configuration information
    // Should be in the following form: "/Projects/"
    strBaseConfigLocation = pConfig->GetPath() + wxT("/");

    // Convert to a zero based index
    iColumnCount = GetColumnCount() - 1;

    // Which fields are we interested in?
    liColumnInfo.SetMask(
        wxLIST_MASK_TEXT |
        wxLIST_MASK_WIDTH |
        wxLIST_MASK_FORMAT
    );

    // Cycle through the columns recording anything interesting
    for (iIndex = 0; iIndex <= iColumnCount; iIndex++) {
        GetColumn(iIndex, liColumnInfo);

        pConfig->SetPath(strBaseConfigLocation + liColumnInfo.GetText());

        pConfig->Write(wxT("Width"), liColumnInfo.GetWidth());
        
#if (defined(__WXMAC__) &&  wxCHECK_VERSION(2,8,0))
        pConfig->Write(wxT("Width"), GetColumnWidth(iIndex)); // Work around bug in wxMac-2.8.0 wxListCtrl::SetColumn()
#endif
    }

    // Save sorting column and direction
    pConfig->SetPath(strBaseConfigLocation);
    pConfig->Write(wxT("SortColumn"), m_pParentView->m_iSortColumn);
    pConfig->Write(wxT("ReverseSortOrder"), m_pParentView->m_bReverseSort);

    return true;
}


bool CBOINCListCtrl::OnRestoreState(wxConfigBase* pConfig) {
    wxString    strBaseConfigLocation = wxEmptyString;
    wxListItem  liColumnInfo;
    wxInt32     iIndex = 0;
    wxInt32     iColumnCount = 0;
    wxInt32     iTempValue = 0;


    wxASSERT(pConfig);


    // Retrieve the base location to store configuration information
    // Should be in the following form: "/Projects/"
    strBaseConfigLocation = pConfig->GetPath() + wxT("/");

    // Convert to a zero based index
    iColumnCount = GetColumnCount() - 1;

    // Which fields are we interested in?
    liColumnInfo.SetMask(
        wxLIST_MASK_TEXT | wxLIST_MASK_WIDTH | wxLIST_MASK_FORMAT
    );

    // Cycle through the columns recording anything interesting
    for (iIndex = 0; iIndex <= iColumnCount; iIndex++) {
        GetColumn(iIndex, liColumnInfo);

        pConfig->SetPath(strBaseConfigLocation + liColumnInfo.GetText());

        pConfig->Read(wxT("Width"), &iTempValue, -1);
        if (-1 != iTempValue) {
            liColumnInfo.SetWidth(iTempValue);
#if (defined(__WXMAC__) &&  wxCHECK_VERSION(2,8,0))
            SetColumnWidth(iIndex,iTempValue); // Work around bug in wxMac-2.8.0 wxListCtrl::SetColumn()
#endif
        }

        pConfig->Read(wxT("Format"), &iTempValue, -1);
        if (-1 != iTempValue) {
            liColumnInfo.SetAlign((wxListColumnFormat)iTempValue);
        }

        SetColumn(iIndex, liColumnInfo);
    }

    // Restore sorting column and direction
    pConfig->SetPath(strBaseConfigLocation);
    pConfig->Read(wxT("ReverseSortOrder"), &iTempValue,-1);
    if (-1 != iTempValue) {
            m_pParentView->m_bReverseSort = iTempValue != 0 ? true : false;
    }
    pConfig->Read(wxT("SortColumn"), &iTempValue,-1);
    if (-1 != iTempValue) {
            m_pParentView->m_iSortColumn = iTempValue;
            m_pParentView->InitSort();
    }

    return true;
}


void CBOINCListCtrl::SelectRow(int row, bool setSelected) {
    SetItemState(row,  setSelected ? wxLIST_STATE_SELECTED : 0, wxLIST_STATE_SELECTED);
}


void CBOINCListCtrl::AddPendingProgressBar(int row) {
    bool duplicate = false;
    int n = (int)m_iRowsNeedingProgressBars.GetCount();
    for (int i=0; i<n; ++i) {
        if (m_iRowsNeedingProgressBars[i] == row) {
            duplicate = true;
        }
    }
    if (!duplicate) {
        m_iRowsNeedingProgressBars.Add(row);
    }
}


wxString CBOINCListCtrl::OnGetItemText(long item, long column) const {
    wxASSERT(m_pParentView);
    wxASSERT(wxDynamicCast(m_pParentView, CBOINCBaseView));

    return m_pParentView->FireOnListGetItemText(item, column);
}


int CBOINCListCtrl::OnGetItemImage(long item) const {
    wxASSERT(m_pParentView);
    wxASSERT(wxDynamicCast(m_pParentView, CBOINCBaseView));

    return m_pParentView->FireOnListGetItemImage(item);
}


void CBOINCListCtrl::DrawProgressBars()
{
    long topItem, numItems, numVisibleItems, row;
    wxRect r, rr;
    int w = 0, x = 0, xx, yy, ww;
    int progressColumn = m_pParentView->GetProgressColumn();
    
#if USE_NATIVE_LISTCONTROL
    wxClientDC dc(this);
    m_bProgressBarEventPending = false;
#else
    wxClientDC dc(GetMainWin());   // Available only in wxGenericListCtrl
#endif

    if (progressColumn < 0) {
        m_iRowsNeedingProgressBars.Clear();
        return;
    }

    int n = (int)m_iRowsNeedingProgressBars.GetCount();
    if (n <= 0) return;
    
    wxColour progressColor = wxTheColourDatabase->Find(wxT("LIGHT BLUE"));
    wxBrush progressBrush(progressColor);
    
    numItems = GetItemCount();
    if (numItems) {
        topItem = GetTopItem();     // Doesn't work properly for Mac Native control in wxMac-2.8.7

        numVisibleItems = GetCountPerPage();
        ++numVisibleItems;

        if (numItems <= (topItem + numVisibleItems)) numVisibleItems = numItems - topItem;

        x = 0;
        for (int i=0; i< progressColumn; i++) {
            x += GetColumnWidth(i);
        }
        w = GetColumnWidth(progressColumn);
        
#if USE_NATIVE_LISTCONTROL
        x -= GetScrollPos(wxHORIZONTAL);
#else
        CalcScrolledPosition(x, 0, &x, &yy);
#endif
        wxFont theFont = GetFont();
        dc.SetFont(theFont);
        
        for (int i=0; i<n; ++i) {
            row = m_iRowsNeedingProgressBars[i];
            if (row < topItem) continue;
            if (row > (topItem + numVisibleItems -1)) continue;
        

            GetItemRect(row, r);
#if ! USE_NATIVE_LISTCONTROL
            r.y = r.y - GetHeaderHeight() - 1;
#endif
            r.x = x;
            r.width = w;
            r.Inflate(-1, -2);
            rr = r;

            wxString progressString = m_pParentView->GetProgressText(row);
            dc.GetTextExtent(progressString, &xx, &yy);
            
            r.y += (r.height - yy - 1) / 2;
            
            // Adapted from ellipis code in wxRendererGeneric::DrawHeaderButtonContents()
            if (xx > r.width) {
                int ellipsisWidth;
                dc.GetTextExtent( wxT("..."), &ellipsisWidth, NULL);
                if (ellipsisWidth > r.width) {
                    progressString.Clear();
                    xx = 0;
                } else {
                    do {
                        progressString.Truncate( progressString.length() - 1 );
                        dc.GetTextExtent( progressString, &xx, &yy);
                    } while (xx + ellipsisWidth > r.width && progressString.length() );
                    progressString.append( wxT("...") );
                    xx += ellipsisWidth;
                }
            }
            
            dc.SetLogicalFunction(wxCOPY);
            dc.SetBackgroundMode(wxSOLID);
            dc.SetPen(progressColor);
            dc.SetBrush(progressBrush);
            dc.DrawRectangle( rr );

            rr.Inflate(-2, -1);
            ww = rr.width * m_pParentView->GetProgressValue(row);
            rr.x += ww;
            rr.width -= ww;

#if 0
            // Show background stripes behind progress bars
            wxListItemAttr* attr = m_pParentView->FireOnListGetItemAttr(row);
            wxColour bkgd = attr->GetBackgroundColour();
            dc.SetPen(bkgd);
            dc.SetBrush(bkgd);
#else
            dc.SetPen(*wxWHITE_PEN);
            dc.SetBrush(*wxWHITE_BRUSH);
#endif
            dc.DrawRectangle( rr );

            dc.SetPen(*wxBLACK_PEN);
            dc.SetBackgroundMode(wxTRANSPARENT);
            if (xx > (r.width - 7)) {
                dc.DrawText(progressString, r.x, r.y);
            } else {
                dc.DrawText(progressString, r.x + (w - 8 - xx), r.y);
            }
        }
    }
    m_iRowsNeedingProgressBars.Clear();
}

#if USE_NATIVE_LISTCONTROL

void MyEvtHandler::OnPaint(wxPaintEvent & event)
{
    event.Skip();
    if (m_listCtrl) {
        m_listCtrl->PostDrawProgressBarEvent();
    }
}

void CBOINCListCtrl::PostDrawProgressBarEvent() {
    if (m_bProgressBarEventPending) return;
    
    CDrawProgressBarEvent newEvent(wxEVT_DRAW_PROGRESSBAR, this);
    AddPendingEvent(newEvent);
    m_bProgressBarEventPending = true;
}

void CBOINCListCtrl::OnDrawProgressBar(CDrawProgressBarEvent& event) {
    DrawProgressBars();
    event.Skip();
}

#else

void MyEvtHandler::OnPaint(wxPaintEvent & event)
{
    if (m_listCtrl) {
        m_listCtrl->savedHandler->ProcessEvent(event);
        m_listCtrl->DrawProgressBars();
    } else {
        event.Skip();
    }
}

#endif


#if ! USE_LIST_CACHE_HINT

// Work around features in multiple selection virtual wxListCtrl:
//  * It does not send deselection events (except ctrl-click).
//  * It does not send selection events if you add to selection
//    using Shift_Click.
//
// Post a special event.  This will allow this mouse event to
// propogate through the chain to complete any selection or
// deselection operatiion, then the special event will trigger
// CBOINCBaseView::OnCheckSelectionChanged() to respond to the
// selection change, if any.
//
void CBOINCListCtrl::OnMouseDown(wxMouseEvent& event) {
    CCheckSelectionChangedEvent newEvent(wxEVT_CHECK_SELECTION_CHANGED, this);
    m_pParentView->GetEventHandler()->AddPendingEvent(newEvent);
    event.Skip();
}

#endif


// To reduce flicker, refresh only changed columns (except 
// on Mac, which is double-buffered to eliminate flicker.)
void CBOINCListCtrl::RefreshCell(int row, int col) {
    wxRect r;
    
#if (defined (__WXMSW__) && wxCHECK_VERSION(2,8,0))
    GetSubItemRect(row, col, r);
#else
    int i;
    
    GetItemRect(row, r);
#if ! USE_NATIVE_LISTCONTROL
    r.y = r.y - GetHeaderHeight() - 1;
#endif
    for (i=0; i< col; i++) {
        r.x += GetColumnWidth(i);
    }
    r.width = GetColumnWidth(col);
#endif

    RefreshRect(r);
}


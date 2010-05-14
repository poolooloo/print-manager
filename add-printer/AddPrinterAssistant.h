/***************************************************************************
 *   Copyright (C) 2010 by Daniel Nicoletti                                *
 *   dantti85-pk@yahoo.com.br                                              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; see the file COPYING. If not, write to       *
 *   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,  *
 *   Boston, MA 02110-1301, USA.                                           *
 ***************************************************************************/

#ifndef ADD_PRINTER_ASSISTANT_H
#define ADD_PRINTER_ASSISTANT_H

#include <KAssistantDialog>

class AddPrinterAssistant : public KAssistantDialog
{
    Q_OBJECT
public:
    AddPrinterAssistant();
    virtual ~AddPrinterAssistant();

public slots:
    void back();
    void next();
    void enableNextButton(bool enable);
    void enableFinishButton(bool enable);

private:
    void next(KPageWidgetItem *currentPage);
    void setCurrentPage(KPageWidgetItem *page);
    KPageWidgetItem *m_introPage;
    KPageWidgetItem *m_devicesPage;
    KPageWidgetItem *m_uriPage;
    KPageWidgetItem *m_serialPage;
    KPageWidgetItem *m_addPrinterPage;
};

#endif

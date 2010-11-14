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

#include "PageChoosePPD.h"

#include "DevicesModel.h"

#include <QPainter>
#include <KDebug>

PageChoosePPD::PageChoosePPD(QWidget *parent)
 : GenericPage(parent),
   m_isValid(false)
{
    setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);

    // setup default options
    setWindowTitle(i18nc("@title:window", "Select a Printer to Add"));
    // loads the standard key icon
    QPixmap pixmap;
    pixmap = KIconLoader::global()->loadIcon("printer",
                                             KIconLoader::NoGroup,
                                             KIconLoader::SizeEnormous, // a not so huge icon
                                             KIconLoader::DefaultState);
    QPixmap icon(pixmap);
    QPainter painter(&icon);

    pixmap = KIconLoader::global()->loadIcon("page-zoom",
                                             KIconLoader::NoGroup,
                                             KIconLoader::SizeLarge, // a not so huge icon
                                             KIconLoader::DefaultState);
    // the the emblem icon to size 32
    int overlaySize = KIconLoader::SizeLarge;
    QPoint startPoint;
    // bottom right corner
    startPoint = QPoint(KIconLoader::SizeEnormous - overlaySize - 2,
                        KIconLoader::SizeEnormous - overlaySize - 2);
    painter.drawPixmap(startPoint, pixmap);
    printerL->setPixmap(icon);


    m_layout = new QStackedLayout;
    m_layout->setContentsMargins(0, 0, 0, 0);
    gridLayout->addLayout(m_layout, 1, 3);
    m_selectMM = new SelectMakeModel(this);
    m_layout->addWidget(m_selectMM);

}

PageChoosePPD::~PageChoosePPD()
{
}

void PageChoosePPD::setValues(const QHash<QString, QVariant> &args)
{
    m_args = args;
    kDebug() << args;
    if (args["add-new-printer"].toBool()) {
        connect(m_selectMM, SIGNAL(changed(bool)),
                this, SLOT(checkSelected()));
        m_selectMM->setMakeModel(QString(), QString());
        m_isValid = true;
    } else {
        m_isValid = false;
    }
}

bool PageChoosePPD::isValid() const
{
    return m_isValid;
}

bool PageChoosePPD::hasChanges() const
{
    if (!isValid()) {
        return false;
    }

    QString deviceURI;
    if (canProceed()) {
//         deviceURI = devicesLV->selectionModel()->selectedIndexes().first().data(DevicesModel::DeviceURI).toString();
    }
    return deviceURI != m_args["device-uri"];
}

QHash<QString, QVariant> PageChoosePPD::values() const
{
    if (!isValid()) {
        return m_args;
    }

    QHash<QString, QVariant> ret = m_args;
    if (canProceed()) {
        if (originCB->currentIndex() == 0) {
            QString makeAndModel = m_selectMM->selectedMakeAndModel();
            QString ppdName = m_selectMM->selectedPPDName();
            if (!ppdName.isEmpty() && !makeAndModel.isEmpty()){
                ret["ppd-name"] = ppdName;
            }
        }
    }
    return ret;
}

bool PageChoosePPD::canProceed() const
{
    // It can proceed if a PPD file (local or not) is provided    bool changed = false;
    bool allow = false;
    if (originCB->currentIndex() == 0) {
        QString makeAndModel = m_selectMM->selectedMakeAndModel();
        QString ppdName = m_selectMM->selectedPPDName();
        kDebug() << ppdName << makeAndModel;
        if (!ppdName.isEmpty() && !makeAndModel.isEmpty()){
            allow = true;
        }
    } else if (originCB->currentIndex() == 0) {
//         fileKUR->button()->click();
//         if (fileKUR->url().isEmpty()) {
//             makeCB->setCurrentIndex(makeCB->property("lastIndex").toInt());
//             return;
//         }
//         emit showKUR();
//         // set the QVariant type to bool makes it possible to know a file was selected
//         m_changedValues["ppd-name"] = true;
    }
    return allow;
}

void PageChoosePPD::checkSelected()
{
    emit allowProceed(canProceed());
}

#include "PageChoosePPD.moc"

/*****************************************************************************
 *
 * This file is part of the 'WebCamForceSettings' project.
 *
 * This project is designed to work with the settings (properties) of the
 * webcam, contains programs that are covered by one license.
 *
 * Copyright (C) 2021 Alexander Tsyganov.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
*****************************************************************************/

#include "fsitemdelegate.h"

#include <QKeyEvent>
#include <QApplication>

FSItemDelegate::FSItemDelegate(QObject *parent) : QItemDelegate(parent)
{

}

void FSItemDelegate::paint(QPainter *painter,
                           const QStyleOptionViewItem &option,
                           const QModelIndex &index) const
{
    if (index.isValid()) {
        QVariant value = index.data(Qt::DisplayRole);

        if (value.type() == QVariant::Bool) {
            Qt::CheckState state = Qt::PartiallyChecked;

            if (!value.isNull()) {
                if (value.toBool())
                    state = Qt::Checked;
                else
                    state = Qt::Unchecked;
            }

            QStyleOptionViewItem checkBoxStyle(option);
            checkBoxStyle.displayAlignment = Qt::AlignCenter;
            checkBoxStyle.rect = doCheck(checkBoxStyle, checkBoxStyle.rect);

            drawBackground(painter, option, index);
            drawCheck(painter, checkBoxStyle, checkBoxStyle.rect, state);
            drawFocus(painter, option, option.rect);
            return;
        }
    }

    QItemDelegate::paint(painter, option, index);
}

QWidget *FSItemDelegate::createEditor(QWidget *parent,
                                      const QStyleOptionViewItem &option,
                                      const QModelIndex &index) const
{
    if (index.isValid() && index.data(Qt::DisplayRole).type() == QVariant::Bool)
        return nullptr;

    return QItemDelegate::createEditor(parent, option, index);
}

QRect FSItemDelegate::doCheck(const QStyleOptionViewItem &option,
                              const QRect &bounding) const
{
    QRect rect = QItemDelegate::doCheck(option, bounding, Qt::Unchecked);

    // Horizontal
    if (option.displayAlignment & Qt::AlignLeft)
        rect.moveLeft(bounding.x() + QApplication::style()->pixelMetric(QStyle::PM_FocusFrameHMargin) + 1);
    else if (option.displayAlignment & Qt::AlignHCenter)
        rect.moveLeft(bounding.center().x() - rect.width() / 2);
    else if (option.displayAlignment & Qt::AlignRight)
        rect.moveLeft(bounding.right() - rect.width() - QApplication::style()->pixelMetric(QStyle::PM_FocusFrameHMargin) - 1);

    // Vertical
    if (option.displayAlignment & Qt::AlignTop)
        rect.moveTop(bounding.y() + QApplication::style()->pixelMetric(QStyle::PM_FocusFrameVMargin) + 1);
    else if (option.displayAlignment & Qt::AlignVCenter)
        rect.moveTop(bounding.center().y() - rect.height() / 2);
    else if (option.displayAlignment & Qt::AlignBottom)
        rect.moveTop(bounding.bottom() - rect.height() - QApplication::style()->pixelMetric(QStyle::PM_FocusFrameVMargin) - 1);

    return rect;
}

bool FSItemDelegate::editorEvent(QEvent *event,
                                 QAbstractItemModel *model,
                                 const QStyleOptionViewItem &option,
                                 const QModelIndex &index)
{
    if (index.isValid()) {
        const QVariant value = index.data(Qt::DisplayRole);
        if (value.type() == QVariant::Bool) {
            if (event && model) {
                const Qt::ItemFlags flags = model->flags(index);
                if ((flags & Qt::ItemIsUserCheckable) || (flags & Qt::ItemIsEnabled)) {
                    if ( event->type() == QEvent::MouseButtonRelease ||
                         event->type() == QEvent::MouseButtonPress ) {
                        QMouseEvent *mouseEvent = (QMouseEvent*)event;

                        // Fix aligment option
                        QStyleOptionViewItem fixedOption(option);
                        const QVariant textAlignmentValue = index.data(Qt::TextAlignmentRole);
                        if (!textAlignmentValue.isNull()) {
                            Qt::Alignment textAlignment = Qt::Alignment(textAlignmentValue.value<int>());

                            if (fixedOption.displayAlignment != textAlignment) {
                                fixedOption.displayAlignment = textAlignment;
                            }
                        }

                        if (doCheck(fixedOption, fixedOption.rect).contains(mouseEvent->pos())) {
                            if (event->type() == QEvent::MouseButtonPress)
                                return true;

                            if (!value.isNull())
                                return model->setData(index, !value.toBool());
                            else
                                return false;
                        }
                    }
                    else if (event->type() == QEvent::KeyPress) {
                        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);

                        if ( keyEvent->key() == Qt::Key_Space ||
                             keyEvent->key() == Qt::Key_Select )
                        {
                            if (!value.isNull())
                                return model->setData(index, !value.toBool());
                            else
                                return false;
                        }
                    }
                }
            }
        }
    }

    return QItemDelegate::editorEvent(event, model, option, index);
}

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

#include "fsiconcreator.h"

#include <QtMath>
#include <QPainter>

QPixmap FSIconCreator::generate(int size)
{
    const int hypotenuse = qSqrt(size * size + size * size);

    QPixmap pixmap(size, size);
    pixmap.fill(Qt::transparent);

    QColor backgroundColor = Qt::white;
    QColor frontgroundColor = Qt::black;

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing, true);

    painter.setPen(backgroundColor);
    painter.setBrush(backgroundColor);

    // Draw background circle
    double marginScale = 0.01;
    painter.drawEllipse(size * marginScale,
                        size * marginScale,
                        size * (1 - 2 * marginScale),
                        size * (1 - 2 * marginScale));

    painter.setPen(frontgroundColor);
    painter.setBrush(frontgroundColor);

    // Draw webcam head
    double camHeadYOffsetScale = 0.3;
    double camHeadWidthScale = 0.8;
    double camHeadHeightScale = 0.32;
    double camHeadRoundedScale = 0.05;
    painter.drawRoundedRect(size * (0.5 - camHeadWidthScale / 2),
                            size * camHeadYOffsetScale,
                            size * camHeadWidthScale,
                            size * camHeadHeightScale,
                            hypotenuse * camHeadRoundedScale,
                            hypotenuse * camHeadRoundedScale);

    // Draw webcam leg
    double camLegWidthScale = 0.125;
    double camLegHeightScale = 0.075;
    painter.drawRect(size * (0.5 - camLegWidthScale / 2),
                     size * (camHeadYOffsetScale + camHeadHeightScale),
                     size * camLegWidthScale,
                     qRound(size * camLegHeightScale));

    // Draw webcam footing
    double camFootingWidthScale = 0.5;
    double camFootingHeightScale = 0.1;
    double camFootingRoundedScale = 0.025;
    painter.drawRoundedRect(size * (0.5 - camFootingWidthScale / 2),
                            size * (camHeadYOffsetScale + camHeadHeightScale + camLegHeightScale),
                            size * camFootingWidthScale,
                            size * camFootingHeightScale,
                            hypotenuse * camFootingRoundedScale,
                            hypotenuse * camFootingRoundedScale);
    painter.drawRoundedRect(size * (0.5 - camFootingWidthScale / 2),
                            size * (camHeadYOffsetScale + camHeadHeightScale + camLegHeightScale + camFootingHeightScale / 2.5),
                            size * camFootingWidthScale,
                            size * (camFootingHeightScale - camFootingHeightScale / 2.5),
                            hypotenuse * (camFootingRoundedScale * 0.375),
                            hypotenuse * (camFootingRoundedScale * 0.375));

    // Draw webcam border lens
    double camBorderLensWidthScale = 0.0275;
    double camBorderLensRadiusScale = 0.1;
    painter.setPen(QPen(backgroundColor, hypotenuse * camBorderLensWidthScale));
    painter.setBrush(QBrush());
    painter.drawEllipse(QPointF(size * 0.5,
                                size * (camHeadYOffsetScale + camHeadHeightScale / 2)),
                        size * camBorderLensRadiusScale,
                        size * camBorderLensRadiusScale);

    painter.setPen(backgroundColor);
    painter.setBrush(backgroundColor);

    // Draw lens flare
    double camFlareRadiusScale = 0.02;
    double camFlareXOffsetScale = 0.7;
    double camFlareYOffsetScale = 0.3;
    painter.drawEllipse(QPointF(size * (0.5 - camBorderLensRadiusScale + camFlareXOffsetScale * 2 * camBorderLensRadiusScale),
                                size * (camHeadYOffsetScale + camHeadHeightScale / 2 - camBorderLensRadiusScale + camFlareYOffsetScale * 2 * camBorderLensRadiusScale)),
                        size * camFlareRadiusScale,
                        size * camFlareRadiusScale);

    return pixmap;
}

FSIconCreator::FSIconCreator()
{
    // do nothing
}

FSIconCreator::~FSIconCreator()
{
    // do nothing
}

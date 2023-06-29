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

#include "fscamera.h"

#include <windows.h>
#include <dshow.h>

#include <strmif.h>
#include <olectl.h>

#include <ks.h>
#include <ksmedia.h>

#include <map>

#include <QObject>

#include <WebCamFS/Settings>
#include <WebCamFS/CoreCamerasStorage>

#define INFORMATIVE_IUNKNOWN_RELEASE(object) \
    FSCameraPrivate::informativeIUnknownRelease(object, __LINE__)

class FSCameraPrivate
{
public:
    FSCameraPrivate();

    FSCoreCamerasStorage *coreCamerasStorage;

    FSCameraData cameraData;

    enum PropertyInterface
    {
        VideoProcAmpInterface = 0,
        CameraControlInterface
    };

    static PropertyInterface getPropertyInterface(FSCameraProperty property);

    static LONG getProperty(FSCameraProperty property);

    static HRESULT initCaptureGraphBuilder(IGraphBuilder **ppGraph,
                                           ICaptureGraphBuilder2 **ppBuild);

    static HRESULT enumerateDevices(IEnumMoniker **ppEnum);

    static DeviceName getDeviceName(IPropertyBag *pPropBag);
    static DevicePath getDevicePath(IPropertyBag *pPropBag);

    static bool propertiesIsAvailable(IGraphBuilder *pGraph, IBaseFilter *pCap);

    static inline void informativeIUnknownRelease(IUnknown *object, uint line);
};

FSCameraPrivate::FSCameraPrivate()
    : coreCamerasStorage(nullptr)
{
    // do nothing
}

FSCameraPrivate::PropertyInterface FSCameraPrivate::getPropertyInterface(FSCameraProperty property)
{
    switch (property) {
    case FSCameraProperty::None:
    case FSCameraProperty::Brightness:
    case FSCameraProperty::Contrast:
    case FSCameraProperty::Hue:
    case FSCameraProperty::Saturation:
    case FSCameraProperty::Sharpness:
    case FSCameraProperty::Gamma:
    case FSCameraProperty::ColorEnable:
    case FSCameraProperty::WhiteBalance:
    case FSCameraProperty::BacklightCompensation:
    case FSCameraProperty::Gain:
    case FSCameraProperty::PowerlineFrequency:
        return VideoProcAmpInterface;
    case FSCameraProperty::Pan:
    case FSCameraProperty::Tilt:
    case FSCameraProperty::Roll:
    case FSCameraProperty::Zoom:
    case FSCameraProperty::Exposure:
    case FSCameraProperty::Iris:
    case FSCameraProperty::Focus:
        return CameraControlInterface;
    }

    return VideoProcAmpInterface;
}

LONG FSCameraPrivate::getProperty(FSCameraProperty property)
{
    switch (property) {
    case FSCameraProperty::None:
        return -1;
    case FSCameraProperty::Brightness:
        return VideoProcAmp_Brightness;
    case FSCameraProperty::Contrast:
        return VideoProcAmp_Contrast;
    case FSCameraProperty::Hue:
        return VideoProcAmp_Hue;
    case FSCameraProperty::Saturation:
        return VideoProcAmp_Saturation;
    case FSCameraProperty::Sharpness:
        return VideoProcAmp_Sharpness;
    case FSCameraProperty::Gamma:
        return VideoProcAmp_Gamma;
    case FSCameraProperty::ColorEnable:
        return VideoProcAmp_ColorEnable;
    case FSCameraProperty::WhiteBalance:
        return VideoProcAmp_WhiteBalance;
    case FSCameraProperty::BacklightCompensation:
        return VideoProcAmp_BacklightCompensation;
    case FSCameraProperty::Gain:
        return VideoProcAmp_Gain;
    case FSCameraProperty::PowerlineFrequency:
        return KSPROPERTY_VIDEOPROCAMP_POWERLINE_FREQUENCY;
    case FSCameraProperty::Pan:
        return CameraControl_Pan;
    case FSCameraProperty::Tilt:
        return CameraControl_Tilt;
    case FSCameraProperty::Roll:
        return CameraControl_Roll;
    case FSCameraProperty::Zoom:
        return CameraControl_Zoom;
    case FSCameraProperty::Exposure:
        return CameraControl_Exposure;
    case FSCameraProperty::Iris:
        return CameraControl_Iris;
    case FSCameraProperty::Focus:
        return CameraControl_Focus;
    }

    return -1;
}

HRESULT FSCameraPrivate::initCaptureGraphBuilder(IGraphBuilder **ppGraph,
                                                 ICaptureGraphBuilder2 **ppBuild)
{
    if (!ppGraph || !ppBuild) {
        return E_POINTER;
    }

    IGraphBuilder *pGraph = nullptr;
    ICaptureGraphBuilder2 *pBuild = nullptr;

    HRESULT hr = CoCreateInstance(CLSID_CaptureGraphBuilder2,
                                  nullptr,
                                  CLSCTX_INPROC_SERVER,
                                  IID_ICaptureGraphBuilder2,
                                  (void**)&pBuild);

    if (hr == S_OK) {
        hr = CoCreateInstance(CLSID_FilterGraph,
                              nullptr,
                              CLSCTX_INPROC_SERVER,
                              IID_IGraphBuilder,
                              (void**)&pGraph);

        if (hr == S_OK) {
            hr = pBuild->SetFiltergraph(pGraph);

            if (hr == S_OK) {
                *ppBuild = pBuild;
                *ppGraph = pGraph;

                return S_OK;
            } else {
                qWarning("Unexpected return value (%ld) for \"ICaptureGraphBuilder2::SetFiltergraph(IGraphBuilder *)\" function!", hr);

                INFORMATIVE_IUNKNOWN_RELEASE(pBuild);
                INFORMATIVE_IUNKNOWN_RELEASE(pGraph);
            }

            return hr;
        }
        else {
            qWarning("Unexpected return value (%ld) for \"CoCreateInstance(CLSID_FilterGraph, LPUNKNOWN, DWORD, IID_IGraphBuilder, LPVOID *)\" function!", hr);

            INFORMATIVE_IUNKNOWN_RELEASE(pBuild);
        }
    } else {
        qWarning("Unexpected return value (%ld) for \"CoCreateInstance(CLSID_CaptureGraphBuilder2, LPUNKNOWN, DWORD, IID_ICaptureGraphBuilder2, LPVOID *)\" function!", hr);
    }

    return hr;
}

HRESULT FSCameraPrivate::enumerateDevices(IEnumMoniker **ppEnum)
{
    ICreateDevEnum *pDevEnum;

    HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum,
                                  nullptr,
                                  CLSCTX_INPROC_SERVER,
                                  IID_PPV_ARGS(&pDevEnum));

    if (hr == S_OK) {
        hr = pDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, ppEnum, 0);
        INFORMATIVE_IUNKNOWN_RELEASE(pDevEnum);

        if (hr != S_OK && hr != S_FALSE) {
            qWarning("Unexpected return value (%ld) for \"ICreateDevEnum::CreateClassEnumerator(CLSID_VideoInputDeviceCategory, IEnumMoniker **, DWORD)\" function!", hr);
        }
    } else {
        qWarning("Unexpected return value (%ld) for \"CoCreateInstance(CLSID_SystemDeviceEnum, LPUNKNOWN, DWORD, REFIID, LPVOID *)\" function!", hr);
    }

    return hr;
}

DeviceName FSCameraPrivate::getDeviceName(IPropertyBag *pPropBag)
{
    DeviceName result;

    VARIANT var;
    VariantInit(&var);

    HRESULT hr = pPropBag->Read(L"Description", &var, nullptr);
    if (hr != S_OK) {
        hr = pPropBag->Read(L"FriendlyName", &var, nullptr);
    }

    if (hr == S_OK) {
        result = QString::fromWCharArray(var.bstrVal);
        VariantClear(&var);
    } else {
        qWarning("Unexpected return value (%ld) for \"IPropertyBag::Read(\"Description\" AND \"FriendlyName\", VARIANT *, IErrorLog *)\" function!", hr);
    }

    return result;
}

DevicePath FSCameraPrivate::getDevicePath(IPropertyBag *pPropBag)
{
    DevicePath result;

    VARIANT var;
    VariantInit(&var);

    HRESULT hr = pPropBag->Read(L"DevicePath", &var, nullptr);
    if (hr == S_OK) {
        result = QString::fromWCharArray(var.bstrVal);
        VariantClear(&var);
    } else {
        qWarning("Unexpected return value (%ld) for \"IPropertyBag::Read(\"DevicePath\", VARIANT *, IErrorLog *)\" function!", hr);
    }

    return result;
}

bool FSCameraPrivate::propertiesIsAvailable(IGraphBuilder *pGraph, IBaseFilter *pCap)
{
    bool propertiesIsAvailable = false;

    HRESULT hr = pGraph->AddFilter(pCap, L"Capture Filter");

    if (hr == S_OK || hr == VFW_S_DUPLICATE_NAME) {
        IAMVideoProcAmp *pProcAmp = nullptr;
        hr = pCap->QueryInterface(IID_IAMVideoProcAmp, (void**)&pProcAmp);
        if (hr == S_OK) {
            propertiesIsAvailable = true;

            INFORMATIVE_IUNKNOWN_RELEASE(pProcAmp);
        } else {
            IAMCameraControl *pCameraControl = nullptr;
            hr = pCap->QueryInterface(IID_IAMCameraControl, (void**)&pCameraControl);
            if (hr == S_OK) {
                propertiesIsAvailable = true;
                INFORMATIVE_IUNKNOWN_RELEASE(pCameraControl);
            }
        }
    } else {
        qWarning("Unexpected return value (%ld) for \"IFilterGraph::AddFilter(IBaseFilter *, LPCWSTR)\" function!", hr);
    }

    return propertiesIsAvailable;
}

void FSCameraPrivate::informativeIUnknownRelease(IUnknown *object, uint line)
{
    const ULONG count = object->Release();

#ifdef SHOW_IUNKNOW_RELEASE_INFO
    if (count != 0) {
        qInfo("Failed to free IUnknown resources at address \"%p\", count=\'%ld\', line=\'%d\'!", object, count, line);
    }
#else
    Q_UNUSED(line);
    Q_UNUSED(count);
#endif // SHOW_IUNKNOW_RELEASE_INFO
}

FSCamera::FSCamera(const FSCameraData &cameraData)
{
    init(cameraData);
}

FSCamera::~FSCamera()
{
    FSCamera::releaseCameraData(d->cameraData);

    delete d;
    d = nullptr;
}

void FSCamera::setCamerasStorage(FSCoreCamerasStorage *coreCamerasStorage)
{
    if (d->coreCamerasStorage != coreCamerasStorage)
    {
        if (d->coreCamerasStorage) {
            d->coreCamerasStorage->unregisterCamera(this);
        }

        d->coreCamerasStorage = coreCamerasStorage;

        if (d->coreCamerasStorage) {
            d->coreCamerasStorage->registerCamera(this);
        }
    }
}

FSCoreCamerasStorage *FSCamera::camerasStorage() const
{
    return d->coreCamerasStorage;
}

bool FSCamera::initializeWinCOMLibrary()
{
    HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);

    switch (hr)
    {
    case S_OK:
        return true;
    case S_FALSE:
        return true;
    case RPC_E_CHANGED_MODE:
        return true;
    default:
        qCritical("Unknow return value (%ld) for \"CoInitializeEx(LPVOID,DWORD)\" function!", hr);
        break;
    }

    return false;
}

void FSCamera::uninitializeWinCOMLibrary()
{
    CoUninitialize();
}

void FSCamera::printCamerasInfo()
{
    IEnumMoniker *pEnumMoniker;
    HRESULT hr = FSCameraPrivate::enumerateDevices(&pEnumMoniker);
    if (hr == S_OK) {
        IMoniker *pMoniker = nullptr;

        IGraphBuilder *pGraph = nullptr;
        ICaptureGraphBuilder2 *pBuild = nullptr;

        hr = FSCameraPrivate::initCaptureGraphBuilder(&pGraph, &pBuild);
        if (hr == S_OK) {
            while (pEnumMoniker->Next(1, &pMoniker, nullptr) == S_OK) {
                IPropertyBag *pPropBag = nullptr;

                hr = pMoniker->BindToStorage(nullptr, nullptr, IID_PPV_ARGS(&pPropBag));
                if (hr != S_OK) {
                    INFORMATIVE_IUNKNOWN_RELEASE(pMoniker);
                    continue;
                }

                printf("Name: \"%s\", DevicePath: \"%s\"\n",
                       FSCameraPrivate::getDeviceName(pPropBag).toLocal8Bit().constData(),
                       FSCameraPrivate::getDevicePath(pPropBag).toLocal8Bit().constData());

                IBaseFilter *pCap = nullptr;
                hr = pMoniker->BindToObject(nullptr, nullptr, IID_IBaseFilter, (void**)&pCap);
                if (hr == S_OK) {
                    hr = pGraph->AddFilter(pCap, L"Capture Filter");

                    if (hr == S_OK || hr == VFW_S_DUPLICATE_NAME) {
                        IAMVideoProcAmp *pProcAmp = nullptr;
                        hr = pCap->QueryInterface(IID_IAMVideoProcAmp, (void**)&pProcAmp);
                        if (hr == S_OK) {
                            LONG vMin, vMax, vStep, vDefault, vFlags, vVal;

                            const std::map<LONG, std::string> mapVideoProcAmpProperties {
                                { VideoProcAmp_Brightness,                     "Brightness"},
                                { VideoProcAmp_Contrast,                       "Contrast"},
                                { VideoProcAmp_Hue,                            "Hue"},
                                { VideoProcAmp_Saturation,                     "Saturation"},
                                { VideoProcAmp_Sharpness,                      "Sharpness"},
                                { VideoProcAmp_Gamma,                          "Gamma"},
                                { VideoProcAmp_ColorEnable,                    "ColorEnable"},
                                { VideoProcAmp_WhiteBalance,                   "WhiteBalance"},
                                { VideoProcAmp_BacklightCompensation,          "BacklightCompensation"},
                                { VideoProcAmp_Gain,                           "Gain"},
                                { KSPROPERTY_VIDEOPROCAMP_POWERLINE_FREQUENCY, "PowerlineFrequency"},
                            };

                            for (const auto& [key, name] : mapVideoProcAmpProperties) {
                                QString spaces;
                                spaces.fill(QChar::Space, (25 - int(name.length())));

                                printf("%s%s: ", spaces.toLatin1().constData(), name.c_str());

                                hr = pProcAmp->GetRange(key, &vMin, &vMax, &vStep, &vDefault, &vFlags);
                                if (hr == S_OK) {
                                    printf("Range(Min: %ld, Max: %ld, Step: %ld, Default: %ld, Flags: %ld), Current(",
                                           vMin,
                                           vMax,
                                           vStep,
                                           vDefault,
                                           vFlags);

                                    hr = pProcAmp->Get(key, &vVal, &vFlags);

                                    if (hr == S_OK) {
                                        printf("Val: %ld, Flags: %ld)\n",
                                               vVal,
                                               vFlags);
                                    }
                                    else {
                                        printf("None)\n");
                                    }
                                }
                                else {
                                    printf("None\n");
                                }
                            }

                            INFORMATIVE_IUNKNOWN_RELEASE(pProcAmp);
                        } else {
                            printf("No video proc amp properties!\n");
                        }

                        IAMCameraControl *pCameraControl = nullptr;
                        hr = pCap->QueryInterface(IID_IAMCameraControl, (void**)&pCameraControl);
                        if (hr == S_OK) {
                            LONG vMin, vMax, vStep, vDefault, vFlags, vVal;

                            const std::map<LONG, std::string> mapCameraControlProperty {
                                { CameraControl_Pan,      "Pan"},
                                { CameraControl_Tilt,     "Tilt"},
                                { CameraControl_Roll,     "Roll"},
                                { CameraControl_Zoom,     "Zoom"},
                                { CameraControl_Exposure, "Exposure"},
                                { CameraControl_Iris,     "Iris"},
                                { CameraControl_Focus,    "Focus"},
                            };

                            for (const auto& [key, name] : mapCameraControlProperty) {
                                QString spaces;
                                spaces.fill(QChar::Space, (25 - int(name.length())));

                                printf("%s%s: ", spaces.toLatin1().constData(), name.c_str());

                                hr = pCameraControl->GetRange(key, &vMin, &vMax, &vStep, &vDefault, &vFlags);
                                if (hr == S_OK) {
                                    printf("Range(Min: %ld, Max: %ld, Step: %ld, Default: %ld, Flags: %ld), Current(",
                                           vMin,
                                           vMax,
                                           vStep,
                                           vDefault,
                                           vFlags);

                                    hr = pCameraControl->Get(key, &vVal, &vFlags);

                                    if (hr == S_OK) {
                                        printf("Val: %ld, Flags: %ld)\n",
                                               vVal,
                                               vFlags);
                                    }
                                    else {
                                        printf("None)\n");
                                    }
                                }
                                else {
                                    printf("None\n");
                                }
                            }

                            INFORMATIVE_IUNKNOWN_RELEASE(pCameraControl);
                        }
                        else {
                            printf("No camera control properties!\n");
                        }
                    } else {
                        printf("Cannot add capture filter!\n");
                    }

                    INFORMATIVE_IUNKNOWN_RELEASE(pCap);
                } else {
                    printf("Cannot create base filter!\n");
                }

                printf("\n");

                INFORMATIVE_IUNKNOWN_RELEASE(pPropBag);
                INFORMATIVE_IUNKNOWN_RELEASE(pMoniker);
            }

            INFORMATIVE_IUNKNOWN_RELEASE(pGraph);
            INFORMATIVE_IUNKNOWN_RELEASE(pBuild);
        } else {
            printf("Cannot create capture graph builder!\n");
        }

        INFORMATIVE_IUNKNOWN_RELEASE(pEnumMoniker);
    } else {
        if (hr == S_FALSE) {
            printf("No video input devices available!\n");
        }
    }
}

FSCamera *FSCamera::findCameraByDevicePath(const DevicePath &devicePath)
{
    IEnumMoniker *pEnumMoniker;
    HRESULT hr = FSCameraPrivate::enumerateDevices(&pEnumMoniker);
    if (hr == S_OK) {
        IMoniker *pMoniker = nullptr;

        IGraphBuilder *pGraph = nullptr;
        ICaptureGraphBuilder2 *pBuild = nullptr;

        hr = FSCameraPrivate::initCaptureGraphBuilder(&pGraph, &pBuild);
        if (hr == S_OK) {
            while (pEnumMoniker->Next(1, &pMoniker, nullptr) == S_OK) {
                IPropertyBag *pPropBag = nullptr;

                hr = pMoniker->BindToStorage(nullptr, nullptr, IID_PPV_ARGS(&pPropBag));
                if (hr != S_OK) {
                    INFORMATIVE_IUNKNOWN_RELEASE(pMoniker);
                    continue;
                }

                if (FSCameraPrivate::getDevicePath(pPropBag) == devicePath) {
                    IBaseFilter *pCap = nullptr;
                    hr = pMoniker->BindToObject(nullptr, nullptr, IID_IBaseFilter, (void**)&pCap);
                    if (hr == S_OK) {
                        if (FSCameraPrivate::propertiesIsAvailable(pGraph, pCap)) {
                            FSCameraData cameraData(FSCameraPrivate::getDeviceName(pPropBag),
                                                    devicePath,
                                                    pCap);

                            INFORMATIVE_IUNKNOWN_RELEASE(pPropBag);
                            INFORMATIVE_IUNKNOWN_RELEASE(pMoniker);
                            INFORMATIVE_IUNKNOWN_RELEASE(pGraph);
                            INFORMATIVE_IUNKNOWN_RELEASE(pBuild);
                            INFORMATIVE_IUNKNOWN_RELEASE(pEnumMoniker);

                            return new FSCamera(cameraData);
                        } else {
                            INFORMATIVE_IUNKNOWN_RELEASE(pCap);
                        }
                    }
                }

                INFORMATIVE_IUNKNOWN_RELEASE(pPropBag);
                INFORMATIVE_IUNKNOWN_RELEASE(pMoniker);
            }

            INFORMATIVE_IUNKNOWN_RELEASE(pGraph);
            INFORMATIVE_IUNKNOWN_RELEASE(pBuild);
        }

        INFORMATIVE_IUNKNOWN_RELEASE(pEnumMoniker);
    }

    return nullptr;
}

FSCamerasDataUMap FSCamera::availableCamerasDataUMap()
{
    FSCamerasDataUMap result;
    result.reserve(FSSettings::usuallyAvailableCamerasCount());

    IEnumMoniker *pEnumMoniker;
    HRESULT hr = FSCameraPrivate::enumerateDevices(&pEnumMoniker);
    if (hr == S_OK) {
        IMoniker *pMoniker = nullptr;

        IGraphBuilder *pGraph = nullptr;
        ICaptureGraphBuilder2 *pBuild = nullptr;

        hr = FSCameraPrivate::initCaptureGraphBuilder(&pGraph, &pBuild);
        if (hr == S_OK) {
            while (pEnumMoniker->Next(1, &pMoniker, nullptr) == S_OK) {
                IPropertyBag *pPropBag = nullptr;

                hr = pMoniker->BindToStorage(nullptr, nullptr, IID_PPV_ARGS(&pPropBag));
                if (hr != S_OK) {
                    INFORMATIVE_IUNKNOWN_RELEASE(pMoniker);
                    continue;
                }

                IBaseFilter *pCap = nullptr;
                hr = pMoniker->BindToObject(nullptr, nullptr, IID_IBaseFilter, (void**)&pCap);
                if (hr == S_OK) {
                    if (FSCameraPrivate::propertiesIsAvailable(pGraph, pCap)) {
                        FSCameraData cameraData(FSCameraPrivate::getDeviceName(pPropBag),
                                                FSCameraPrivate::getDevicePath(pPropBag),
                                                pCap);
                        result.insert_or_assign(cameraData.devicePath(), cameraData);
                    } else {
                        INFORMATIVE_IUNKNOWN_RELEASE(pCap);
                    }
                }

                INFORMATIVE_IUNKNOWN_RELEASE(pPropBag);
                INFORMATIVE_IUNKNOWN_RELEASE(pMoniker);
            }

            INFORMATIVE_IUNKNOWN_RELEASE(pGraph);
            INFORMATIVE_IUNKNOWN_RELEASE(pBuild);
        }

        INFORMATIVE_IUNKNOWN_RELEASE(pEnumMoniker);
    }

    return result;
}

void FSCamera::releaseCameraData(FSCameraData &cameraData)
{
    if (cameraData.m_pCap) {
        INFORMATIVE_IUNKNOWN_RELEASE(cameraData.m_pCap);
        cameraData.m_pCap = nullptr;
    }
}

DeviceName FSCamera::name() const
{
    return d->cameraData.name();
}

DevicePath FSCamera::devicePath() const
{
    return d->cameraData.devicePath();
}

DeviceName FSCamera::userName() const
{
    if (d->coreCamerasStorage)
        return d->coreCamerasStorage->getCameraUserName(d->cameraData.devicePath());
    else
        return QString();
}

DeviceName FSCamera::displayName() const
{
    if (d->coreCamerasStorage) {
        return d->coreCamerasStorage->getCameraDisplayName(d->cameraData.devicePath());
    }

    return d->cameraData.name();
}

bool FSCamera::isBlackListed() const
{
    if (d->coreCamerasStorage) {
        return d->coreCamerasStorage->isContaintsBlackList(d->cameraData.devicePath());
    }

    return false;
}

bool FSCamera::getRange(FSCameraProperty property, FSRangeParams &rangeParam) const
{
    bool result = false;

    FSCameraPrivate::PropertyInterface propertyInterface = d->getPropertyInterface(property);
    IUnknown *pPropertyInterface = nullptr;
    HRESULT hr;

    switch (propertyInterface) {
    case FSCameraPrivate::VideoProcAmpInterface:
        hr = d->cameraData.pCap()->QueryInterface(IID_IAMVideoProcAmp,
                                                  (void**)&pPropertyInterface);
        break;
    case FSCameraPrivate::CameraControlInterface:
        hr = d->cameraData.pCap()->QueryInterface(IID_IAMCameraControl,
                                                  (void**)&pPropertyInterface);
        break;
    }

    if (hr == S_OK) {
        LONG vMin, vMax, vStep, vDefault, vFlags;

        switch (propertyInterface) {
        case FSCameraPrivate::VideoProcAmpInterface:
            hr = static_cast<IAMVideoProcAmp *>(pPropertyInterface)->
                    GetRange(FSCameraPrivate::getProperty(property),
                             &vMin,
                             &vMax,
                             &vStep,
                             &vDefault,
                             &vFlags);
            break;
        case FSCameraPrivate::CameraControlInterface:
            hr = static_cast<IAMCameraControl *>(pPropertyInterface)->
                    GetRange(FSCameraPrivate::getProperty(property),
                             &vMin,
                             &vMax,
                             &vStep,
                             &vDefault,
                             &vFlags);
            break;
        }

        if (hr == S_OK) {
            if ((vMin != vMax && vStep != 0)) {
                rangeParam = FSRangeParams(vMin, vMax, vStep, vDefault, vFlags);
                result = true;
            } else {
                qInfo("Invalid range values(Min: %ld, Max: %ld, Step: %ld, Default: %ld, Flags: %ld) are invalid for property \"%s\", devicePath=\"%s\"!",
                      vMin,
                      vMax,
                      vStep,
                      vDefault,
                      vFlags,
                      fsGetEnumName(property).toLocal8Bit().constData(),
                      devicePath().toLocal8Bit().constData());
            }
        }

        INFORMATIVE_IUNKNOWN_RELEASE(pPropertyInterface);
    }

    return result;
}

bool FSCamera::get(FSCameraProperty property, FSValueParams &valueParams) const
{
    bool result = false;

    FSCameraPrivate::PropertyInterface propertyInterface = d->getPropertyInterface(property);
    IUnknown *pPropertyInterface = nullptr;
    HRESULT hr;

    switch (propertyInterface) {
    case FSCameraPrivate::VideoProcAmpInterface:
        hr = d->cameraData.pCap()->QueryInterface(IID_IAMVideoProcAmp, (void**)&pPropertyInterface);
        break;
    case FSCameraPrivate::CameraControlInterface:
        hr = d->cameraData.pCap()->QueryInterface(IID_IAMCameraControl, (void**)&pPropertyInterface);
        break;
    }

    if (hr == S_OK) {
        LONG vValue, vFlags;

        switch (propertyInterface) {
        case FSCameraPrivate::VideoProcAmpInterface:
            hr = static_cast<IAMVideoProcAmp *>(pPropertyInterface)->Get(FSCameraPrivate::getProperty(property), &vValue, &vFlags);
            break;
        case FSCameraPrivate::CameraControlInterface:
            hr = static_cast<IAMCameraControl *>(pPropertyInterface)->Get(FSCameraPrivate::getProperty(property), &vValue, &vFlags);
            break;
        }

        if (hr == S_OK) {
            FSValueParams tmpValueParams(vValue, vFlags);
            valueParams = tmpValueParams;
            result = true;
        }

        INFORMATIVE_IUNKNOWN_RELEASE(pPropertyInterface);
    }

    return result;
}

bool FSCamera::set(FSCameraProperty property, const FSValueParams &valueParams)
{
    if (valueParams.isNull())
        return false;

    bool result = false;

    FSCameraPrivate::PropertyInterface propertyInterface = d->getPropertyInterface(property);
    IUnknown *pPropertyInterface = nullptr;
    HRESULT hr;

    switch (propertyInterface) {
    case FSCameraPrivate::VideoProcAmpInterface:
        hr = d->cameraData.pCap()->QueryInterface(IID_IAMVideoProcAmp, (void**)&pPropertyInterface);
        break;
    case FSCameraPrivate::CameraControlInterface:
        hr = d->cameraData.pCap()->QueryInterface(IID_IAMCameraControl, (void**)&pPropertyInterface);
        break;
    }

    if (hr == S_OK) {
        LONG vValue = valueParams.value();
        LONG vFlags = valueParams.flags();

        switch (propertyInterface) {
        case FSCameraPrivate::VideoProcAmpInterface:
            hr = static_cast<IAMVideoProcAmp *>(pPropertyInterface)->Set(FSCameraPrivate::getProperty(property), vValue, vFlags);
            break;
        case FSCameraPrivate::CameraControlInterface:
            hr = static_cast<IAMCameraControl *>(pPropertyInterface)->Set(FSCameraPrivate::getProperty(property), vValue, vFlags);
            break;
        }

        if (hr == S_OK) {
            result = true;
        }

        INFORMATIVE_IUNKNOWN_RELEASE(pPropertyInterface);
    }

    return result;
}

void FSCamera::init(const FSCameraData &cameraData)
{
    d = new FSCameraPrivate();

    d->cameraData = cameraData;
}

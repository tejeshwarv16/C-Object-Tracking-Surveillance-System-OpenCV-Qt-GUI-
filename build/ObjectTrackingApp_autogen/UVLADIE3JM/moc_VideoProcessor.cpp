/****************************************************************************
** Meta object code from reading C++ file 'VideoProcessor.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.16)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../src/VideoProcessor.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'VideoProcessor.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.16. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_VideoProcessor_t {
    QByteArrayData data[16];
    char stringdata0[209];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_VideoProcessor_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_VideoProcessor_t qt_meta_stringdata_VideoProcessor = {
    {
QT_MOC_LITERAL(0, 0, 14), // "VideoProcessor"
QT_MOC_LITERAL(1, 15, 14), // "frameProcessed"
QT_MOC_LITERAL(2, 30, 0), // ""
QT_MOC_LITERAL(3, 31, 6), // "pixmap"
QT_MOC_LITERAL(4, 38, 13), // "statusUpdated"
QT_MOC_LITERAL(5, 52, 6), // "status"
QT_MOC_LITERAL(6, 59, 17), // "recordingFinished"
QT_MOC_LITERAL(7, 77, 8), // "filePath"
QT_MOC_LITERAL(8, 86, 15), // "startProcessing"
QT_MOC_LITERAL(9, 102, 11), // "deviceIndex"
QT_MOC_LITERAL(10, 114, 14), // "stopProcessing"
QT_MOC_LITERAL(11, 129, 12), // "processFrame"
QT_MOC_LITERAL(12, 142, 21), // "setDrawRestrictedZone"
QT_MOC_LITERAL(13, 164, 7), // "enabled"
QT_MOC_LITERAL(14, 172, 17), // "setDrawTrajectory"
QT_MOC_LITERAL(15, 190, 18) // "setCheckSpeedAlert"

    },
    "VideoProcessor\0frameProcessed\0\0pixmap\0"
    "statusUpdated\0status\0recordingFinished\0"
    "filePath\0startProcessing\0deviceIndex\0"
    "stopProcessing\0processFrame\0"
    "setDrawRestrictedZone\0enabled\0"
    "setDrawTrajectory\0setCheckSpeedAlert"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_VideoProcessor[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      10,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       3,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   64,    2, 0x06 /* Public */,
       4,    1,   67,    2, 0x06 /* Public */,
       6,    1,   70,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       8,    1,   73,    2, 0x0a /* Public */,
       8,    1,   76,    2, 0x0a /* Public */,
      10,    0,   79,    2, 0x0a /* Public */,
      11,    0,   80,    2, 0x0a /* Public */,
      12,    1,   81,    2, 0x0a /* Public */,
      14,    1,   84,    2, 0x0a /* Public */,
      15,    1,   87,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::QPixmap,    3,
    QMetaType::Void, QMetaType::QString,    5,
    QMetaType::Void, QMetaType::QString,    7,

 // slots: parameters
    QMetaType::Void, QMetaType::Int,    9,
    QMetaType::Void, QMetaType::QString,    7,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,   13,
    QMetaType::Void, QMetaType::Bool,   13,
    QMetaType::Void, QMetaType::Bool,   13,

       0        // eod
};

void VideoProcessor::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<VideoProcessor *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->frameProcessed((*reinterpret_cast< QPixmap(*)>(_a[1]))); break;
        case 1: _t->statusUpdated((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 2: _t->recordingFinished((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 3: _t->startProcessing((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 4: _t->startProcessing((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 5: _t->stopProcessing(); break;
        case 6: _t->processFrame(); break;
        case 7: _t->setDrawRestrictedZone((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 8: _t->setDrawTrajectory((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 9: _t->setCheckSpeedAlert((*reinterpret_cast< bool(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (VideoProcessor::*)(QPixmap );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&VideoProcessor::frameProcessed)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (VideoProcessor::*)(QString );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&VideoProcessor::statusUpdated)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (VideoProcessor::*)(QString );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&VideoProcessor::recordingFinished)) {
                *result = 2;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject VideoProcessor::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_VideoProcessor.data,
    qt_meta_data_VideoProcessor,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *VideoProcessor::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *VideoProcessor::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_VideoProcessor.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int VideoProcessor::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 10)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 10;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 10)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 10;
    }
    return _id;
}

// SIGNAL 0
void VideoProcessor::frameProcessed(QPixmap _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void VideoProcessor::statusUpdated(QString _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void VideoProcessor::recordingFinished(QString _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE

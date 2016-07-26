/****************************************************************************
** Meta object code from reading C++ file 'GpioWidget.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.6.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'GpioWidget.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.6.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_debugger__GpioWidget_t {
    QByteArrayData data[11];
    char stringdata0[148];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_debugger__GpioWidget_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_debugger__GpioWidget_t qt_meta_stringdata_debugger__GpioWidget = {
    {
QT_MOC_LITERAL(0, 0, 20), // "debugger::GpioWidget"
QT_MOC_LITERAL(1, 21, 11), // "signalClose"
QT_MOC_LITERAL(2, 33, 0), // ""
QT_MOC_LITERAL(3, 34, 8), // "QWidget*"
QT_MOC_LITERAL(4, 43, 14), // "AttributeType&"
QT_MOC_LITERAL(5, 58, 13), // "slotConfigure"
QT_MOC_LITERAL(6, 72, 14), // "AttributeType*"
QT_MOC_LITERAL(7, 87, 3), // "cfg"
QT_MOC_LITERAL(8, 91, 18), // "slotRepaintByTimer"
QT_MOC_LITERAL(9, 110, 19), // "slotClosingMainForm"
QT_MOC_LITERAL(10, 130, 17) // "slotPollingUpdate"

    },
    "debugger::GpioWidget\0signalClose\0\0"
    "QWidget*\0AttributeType&\0slotConfigure\0"
    "AttributeType*\0cfg\0slotRepaintByTimer\0"
    "slotClosingMainForm\0slotPollingUpdate"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_debugger__GpioWidget[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    2,   39,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       5,    1,   44,    2, 0x08 /* Private */,
       8,    0,   47,    2, 0x08 /* Private */,
       9,    0,   48,    2, 0x08 /* Private */,
      10,    0,   49,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3, 0x80000000 | 4,    2,    2,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 6,    7,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void debugger::GpioWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        GpioWidget *_t = static_cast<GpioWidget *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->signalClose((*reinterpret_cast< QWidget*(*)>(_a[1])),(*reinterpret_cast< AttributeType(*)>(_a[2]))); break;
        case 1: _t->slotConfigure((*reinterpret_cast< AttributeType*(*)>(_a[1]))); break;
        case 2: _t->slotRepaintByTimer(); break;
        case 3: _t->slotClosingMainForm(); break;
        case 4: _t->slotPollingUpdate(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (GpioWidget::*_t)(QWidget * , AttributeType & );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&GpioWidget::signalClose)) {
                *result = 0;
                return;
            }
        }
    }
}

const QMetaObject debugger::GpioWidget::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_debugger__GpioWidget.data,
      qt_meta_data_debugger__GpioWidget,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *debugger::GpioWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *debugger::GpioWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_debugger__GpioWidget.stringdata0))
        return static_cast<void*>(const_cast< GpioWidget*>(this));
    if (!strcmp(_clname, "ISignalListener"))
        return static_cast< ISignalListener*>(const_cast< GpioWidget*>(this));
    if (!strcmp(_clname, "IGuiCmdHandler"))
        return static_cast< IGuiCmdHandler*>(const_cast< GpioWidget*>(this));
    return QWidget::qt_metacast(_clname);
}

int debugger::GpioWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 5)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 5;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 5)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 5;
    }
    return _id;
}

// SIGNAL 0
void debugger::GpioWidget::signalClose(QWidget * _t1, AttributeType & _t2)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_END_MOC_NAMESPACE

/****************************************************************************
** Meta object code from reading C++ file 'RegWidget.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.6.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'RegWidget.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.6.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_debugger__RegWidget_t {
    QByteArrayData data[8];
    char stringdata0[83];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_debugger__RegWidget_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_debugger__RegWidget_t qt_meta_stringdata_debugger__RegWidget = {
    {
QT_MOC_LITERAL(0, 0, 19), // "debugger::RegWidget"
QT_MOC_LITERAL(1, 20, 13), // "signalChanged"
QT_MOC_LITERAL(2, 34, 0), // ""
QT_MOC_LITERAL(3, 35, 11), // "const char*"
QT_MOC_LITERAL(4, 47, 4), // "name"
QT_MOC_LITERAL(5, 52, 8), // "uint64_t"
QT_MOC_LITERAL(6, 61, 3), // "val"
QT_MOC_LITERAL(7, 65, 17) // "slotPollingUpdate"

    },
    "debugger::RegWidget\0signalChanged\0\0"
    "const char*\0name\0uint64_t\0val\0"
    "slotPollingUpdate"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_debugger__RegWidget[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    2,   24,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       7,    2,   29,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3, 0x80000000 | 5,    4,    6,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 3, 0x80000000 | 5,    4,    6,

       0        // eod
};

void debugger::RegWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        RegWidget *_t = static_cast<RegWidget *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->signalChanged((*reinterpret_cast< const char*(*)>(_a[1])),(*reinterpret_cast< uint64_t(*)>(_a[2]))); break;
        case 1: _t->slotPollingUpdate((*reinterpret_cast< const char*(*)>(_a[1])),(*reinterpret_cast< uint64_t(*)>(_a[2]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (RegWidget::*_t)(const char * , uint64_t );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&RegWidget::signalChanged)) {
                *result = 0;
                return;
            }
        }
    }
}

const QMetaObject debugger::RegWidget::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_debugger__RegWidget.data,
      qt_meta_data_debugger__RegWidget,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *debugger::RegWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *debugger::RegWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_debugger__RegWidget.stringdata0))
        return static_cast<void*>(const_cast< RegWidget*>(this));
    return QWidget::qt_metacast(_clname);
}

int debugger::RegWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 2)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 2)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 2;
    }
    return _id;
}

// SIGNAL 0
void debugger::RegWidget::signalChanged(const char * _t1, uint64_t _t2)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_END_MOC_NAMESPACE

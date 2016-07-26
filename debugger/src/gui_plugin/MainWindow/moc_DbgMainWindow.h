/****************************************************************************
** Meta object code from reading C++ file 'DbgMainWindow.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.6.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'DbgMainWindow.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.6.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_debugger__DbgMainWindow_t {
    QByteArrayData data[13];
    char stringdata0[177];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_debugger__DbgMainWindow_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_debugger__DbgMainWindow_t qt_meta_stringdata_debugger__DbgMainWindow = {
    {
QT_MOC_LITERAL(0, 0, 23), // "debugger::DbgMainWindow"
QT_MOC_LITERAL(1, 24, 15), // "signalConfigure"
QT_MOC_LITERAL(2, 40, 0), // ""
QT_MOC_LITERAL(3, 41, 14), // "AttributeType*"
QT_MOC_LITERAL(4, 56, 3), // "cfg"
QT_MOC_LITERAL(5, 60, 19), // "signalRedrawByTimer"
QT_MOC_LITERAL(6, 80, 21), // "signalClosingMainForm"
QT_MOC_LITERAL(7, 102, 15), // "signalWriteUart"
QT_MOC_LITERAL(8, 118, 3), // "buf"
QT_MOC_LITERAL(9, 122, 15), // "slotTimerUpdate"
QT_MOC_LITERAL(10, 138, 18), // "slotUartKeyPressed"
QT_MOC_LITERAL(11, 157, 3), // "str"
QT_MOC_LITERAL(12, 161, 15) // "slotActionAbout"

    },
    "debugger::DbgMainWindow\0signalConfigure\0"
    "\0AttributeType*\0cfg\0signalRedrawByTimer\0"
    "signalClosingMainForm\0signalWriteUart\0"
    "buf\0slotTimerUpdate\0slotUartKeyPressed\0"
    "str\0slotActionAbout"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_debugger__DbgMainWindow[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       7,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       4,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   49,    2, 0x06 /* Public */,
       5,    0,   52,    2, 0x06 /* Public */,
       6,    0,   53,    2, 0x06 /* Public */,
       7,    1,   54,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       9,    0,   57,    2, 0x08 /* Private */,
      10,    1,   58,    2, 0x08 /* Private */,
      12,    0,   61,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3,    4,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,    8,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,   11,
    QMetaType::Void,

       0        // eod
};

void debugger::DbgMainWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        DbgMainWindow *_t = static_cast<DbgMainWindow *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->signalConfigure((*reinterpret_cast< AttributeType*(*)>(_a[1]))); break;
        case 1: _t->signalRedrawByTimer(); break;
        case 2: _t->signalClosingMainForm(); break;
        case 3: _t->signalWriteUart((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 4: _t->slotTimerUpdate(); break;
        case 5: _t->slotUartKeyPressed((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 6: _t->slotActionAbout(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (DbgMainWindow::*_t)(AttributeType * );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&DbgMainWindow::signalConfigure)) {
                *result = 0;
                return;
            }
        }
        {
            typedef void (DbgMainWindow::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&DbgMainWindow::signalRedrawByTimer)) {
                *result = 1;
                return;
            }
        }
        {
            typedef void (DbgMainWindow::*_t)();
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&DbgMainWindow::signalClosingMainForm)) {
                *result = 2;
                return;
            }
        }
        {
            typedef void (DbgMainWindow::*_t)(QString );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&DbgMainWindow::signalWriteUart)) {
                *result = 3;
                return;
            }
        }
    }
}

const QMetaObject debugger::DbgMainWindow::staticMetaObject = {
    { &QMainWindow::staticMetaObject, qt_meta_stringdata_debugger__DbgMainWindow.data,
      qt_meta_data_debugger__DbgMainWindow,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *debugger::DbgMainWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *debugger::DbgMainWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_debugger__DbgMainWindow.stringdata0))
        return static_cast<void*>(const_cast< DbgMainWindow*>(this));
    if (!strcmp(_clname, "IGuiCmdHandler"))
        return static_cast< IGuiCmdHandler*>(const_cast< DbgMainWindow*>(this));
    return QMainWindow::qt_metacast(_clname);
}

int debugger::DbgMainWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 7)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 7;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 7)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 7;
    }
    return _id;
}

// SIGNAL 0
void debugger::DbgMainWindow::signalConfigure(AttributeType * _t1)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void debugger::DbgMainWindow::signalRedrawByTimer()
{
    QMetaObject::activate(this, &staticMetaObject, 1, Q_NULLPTR);
}

// SIGNAL 2
void debugger::DbgMainWindow::signalClosingMainForm()
{
    QMetaObject::activate(this, &staticMetaObject, 2, Q_NULLPTR);
}

// SIGNAL 3
void debugger::DbgMainWindow::signalWriteUart(QString _t1)
{
    void *_a[] = { Q_NULLPTR, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}
QT_END_MOC_NAMESPACE

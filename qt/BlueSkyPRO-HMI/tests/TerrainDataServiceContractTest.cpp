#include "TerrainDataService.h"
#include <QCoreApplication>
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <cassert>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    TerrainDataService service;
    assert(service.state() == QStringLiteral("IDLE"));
    assert(service.completedTiles() == 0);
    assert(service.totalTiles() == 0);
    assert(!service.requestVisualContextTiles(QString(), 10.0, 20.0, 11.0, 21.0,
                                               8, QStringLiteral("key")));
    assert(!service.requestVisualContextTiles(QStringLiteral("mission"), 10.0, 20.0,
                                               11.0, 21.0, 8, QString()));
    assert(!service.requestVisualContextTiles(QStringLiteral("mission"), 10.0, 20.0,
                                               11.0, 21.0, 15, QStringLiteral("key")));
    assert(service.state() == QStringLiteral("IDLE"));
    return 0;
}

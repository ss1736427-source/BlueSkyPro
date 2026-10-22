#include "MissionTemplateResolver.h"

#include <QCoreApplication>
#include <QVariantList>
#include <cassert>

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    MissionTemplateResolver resolver;

    const struct { const char *text; const char *id; } cases[] = {
        {"Картографирование территории", "MT-01"},
        {"Создать 3D модель здания", "MT-02"},
        {"Инспекция моста", "MT-03"},
        {"Мониторинг строительства", "MT-04"},
        {"Патрулирование периметра", "MT-05"},
        {"Поиск пропавшего человека", "MT-06"},
        {"Обнаружить пожар", "MT-07"},
        {"Экологический мониторинг леса", "MT-08"},
        {"Мониторинг посевов", "MT-09"},
        {"Доставка медикаментов", "MT-10"},
        {"Ретрансляция связи в зоне работ", "MT-11"},
        {"Аэрофотосъёмка объекта", "MT-12"},
        {"Обнаружение чужого БПЛА", "MT-13"}
    };

    for (const auto &testCase : cases) {
        const QVariantMap result = resolver.resolve(QString::fromUtf8(testCase.text));
        const QVariantList recommendations = result.value(QStringLiteral("recommendations")).toList();
        bool found = false;
        for (const QVariant &entry : recommendations) {
            if (entry.toMap().value(QStringLiteral("templateId")).toString()
                == QString::fromLatin1(testCase.id))
                found = true;
        }
        assert(found);
        assert(result.value(QStringLiteral("operatorConfirmationRequired")).toBool());
        assert(!result.value(QStringLiteral("flightReleaseEligible")).toBool());
    }

    const QVariantMap empty = resolver.resolve(QString());
    assert(empty.value(QStringLiteral("recommendations")).toList().isEmpty());
    assert(empty.value(QStringLiteral("needsClarification")).toBool());

    const QVariantMap unknown = resolver.resolve(QStringLiteral("Сделать что-нибудь"));
    assert(unknown.value(QStringLiteral("recommendations")).toList().isEmpty());
    assert(unknown.value(QStringLiteral("needsClarification")).toBool());

    const QVariantMap multi = resolver.resolve(QStringLiteral("Сделать картографирование и 3D-модель"));
    assert(multi.value(QStringLiteral("recommendations")).toList().size() >= 2);

    return 0;
}

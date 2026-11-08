#include "MissionTemplateResolver.h"

#include <QCoreApplication>
#include <QVariantList>
#include <cstdio>

namespace {
bool check(bool condition, const char *message)
{
    if (condition)
        return true;
    std::fprintf(stderr, "FAIL: %s\\n", message);
    return false;
}

bool containsTemplate(const QVariantMap &result, const QString &id)
{
    const QVariantList recommendations =
        result.value(QStringLiteral("recommendations")).toList();
    for (const QVariant &entry : recommendations) {
        if (entry.toMap().value(QStringLiteral("templateId")).toString() == id)
            return true;
    }
    return false;
}
}

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    MissionTemplateResolver resolver;
    bool ok = true;

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
        const QString id = QString::fromLatin1(testCase.id);
        ok &= check(containsTemplate(result, id), testCase.text);
        ok &= check(result.value(QStringLiteral("operatorConfirmationRequired")).toBool(),
                    "operator confirmation must remain required");
        ok &= check(!result.value(QStringLiteral("flightReleaseEligible")).toBool(),
                    "lexical recommendation must never release a flight");
    }

    const QVariantMap empty = resolver.resolve(QString());
    ok &= check(empty.value(QStringLiteral("recommendations")).toList().isEmpty(),
                "empty input must not recommend a template");
    ok &= check(empty.value(QStringLiteral("needsClarification")).toBool(),
                "empty input must request clarification");

    const QVariantMap unknown = resolver.resolve(QStringLiteral("Сделать что-нибудь"));
    ok &= check(unknown.value(QStringLiteral("recommendations")).toList().isEmpty(),
                "unknown input must not recommend a template");
    ok &= check(unknown.value(QStringLiteral("needsClarification")).toBool(),
                "unknown input must request clarification");

    const QVariantMap multi =
        resolver.resolve(QStringLiteral("Сделать картографирование и 3D-модель"));
    ok &= check(containsTemplate(multi, QStringLiteral("MT-01")) &&
                containsTemplate(multi, QStringLiteral("MT-02")),
                "compound task must recommend both mapping and 3D reconstruction");

    const QVariantMap negated =
        resolver.resolve(QStringLiteral("Не нужно обнаруживать чужие БПЛА, требуется картографирование территории"));
    ok &= check(!containsTemplate(negated, QStringLiteral("MT-13")),
                "negated C-UAS request must not recommend MT-13");
    ok &= check(containsTemplate(negated, QStringLiteral("MT-01")),
                "positive mapping request must remain recommended beside a negated C-UAS phrase");

    const QVariantMap positiveAfterNegation =
        resolver.resolve(QStringLiteral("Не нужно обнаруживать чужие БПЛА, но требуется обнаружение чужих БПЛА"));
    ok &= check(containsTemplate(positiveAfterNegation, QStringLiteral("MT-13")),
                "a separate positive occurrence must not be suppressed by an earlier negation");

    const QVariantMap genericMention =
        resolver.resolve(QStringLiteral("Картографирование территории рядом с площадкой, где могут находиться БПЛА"));
    ok &= check(!containsTemplate(genericMention, QStringLiteral("MT-13")),
                "a generic mention of UAVs must not trigger C-UAS");

    return ok ? 0 : 1;
}

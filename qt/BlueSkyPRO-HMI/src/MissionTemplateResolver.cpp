#include "MissionTemplateResolver.h"

#include <QRegularExpression>
#include <QStringList>
#include <QVariantList>

namespace {
struct Rule {
    const char *id;
    const char *label;
    const char *patterns;
    const char *reason;
};

const Rule rules[] = {
    {"MT-01", "Картографирование территории", "картограф|карта местност|съёмк[аи] территории|map the area|mapping|terrain mapping", "Указаны картографирование или съёмка территории."},
    {"MT-02", "3D-картография и реконструкция", "3d|тр[её]хмерн|фотограмметр|реконструкц|облачн.{0,12}точек|3d model|reconstruction", "Указаны 3D-модель, фотограмметрия или реконструкция."},
    {"MT-03", "Инспекция объектов и инфраструктуры", "инспекц|обследова.{0,15}(мост|здан|линий|труб|инфраструктур)|осмотр.{0,15}(объект|опор|линий)|infrastructure inspection|inspect the", "Указаны инспекция или обследование объекта."},
    {"MT-04", "Мониторинг строительства", "строительств|стройплощадк|ход.{0,10}работ|прогресс.{0,10}стро|construction progress|construction site", "Указан контроль строительных работ или площадки."},
    {"MT-05", "Мониторинг территории и периметра", "периметр|охрана территории|патрулирован|несанкционирован.{0,15}проникнов|границ.{0,10}участка|perimeter|site security|patrol", "Указаны периметр, патрулирование или охрана территории."},
    {"MT-06", "Поиск и спасение", "поиск.{0,15}(человек|пострадавш|пропавш)|спасательн|поисково-спасательн|sar\b|search and rescue|missing person", "Указаны поиск людей или поисково-спасательная операция."},
    {"MT-07", "Пожарный мониторинг и ЧС", "пожар|задымлен|чрезвычайн.{0,10}ситуац|очаг.{0,10}возгоран|fire detection|wildfire|emergency response", "Указаны пожар, задымление или чрезвычайная ситуация."},
    {"MT-08", "Экологический и природный мониторинг", "эколог|загрязнен|лесн.{0,10}мониторинг|береговой мониторинг|состояни.{0,15}(леса|водо[её]ма|экосистем)|wildlife|environmental monitoring|habitat", "Указаны экологический мониторинг или наблюдение за природой."},
    {"MT-09", "Сельское хозяйство", "сельск.{0,10}хозяйств|агро|урожайн|состояни.{0,10}посев|поле.{0,10}(осмотр|мониторинг)|crop monitoring|precision agriculture", "Указаны сельхозугодья, посевы или агромониторинг."},
    {"MT-10", "Доставка грузов", "доставк.{0,10}(груз|посыл|медикамент|товар)|перевозк.{0,10}груз|cargo delivery|deliver supplies|medical delivery", "Указана доставка или перевозка груза."},
    {"MT-11", "Ретрансляция связи", "ретрансляц.{0,10}связ|усилени.{0,10}сигнал|радиопокрыти|связ.{0,10}в труднодоступ|communication relay|radio relay|network coverage", "Указана ретрансляция связи или расширение радиопокрытия."},
    {"MT-12", "Аэрофотосъёмка и медиапроизводство", "аэрофотос[ъеё]мк|видеос[ъеё]мк|фото.{0,10}с[ъеё]мк|кинематограф|медиапроизводств|aerial photography|cinematic|video shoot", "Указана фото-, видео- или медиасъёмка с воздуха."},
    {"MT-13", "C-UAS — обнаружение БПЛА", "обнаруж.{0,15}(чуж.{0,10}бпла|беспилотник|дрон)|сопровожд.{0,15}(бпла|беспилотник|дрон)|противодейств.{0,15}бпла|c-uas|counter.?uas|detect.{0,15}(drone|uav)|track.{0,15}(drone|uav)", "Явно указаны обнаружение или сопровождение другого БПЛА."}
};
}

MissionTemplateResolver::MissionTemplateResolver(QObject *parent)
    : QObject(parent)
{
}

QVariantMap MissionTemplateResolver::resolve(const QString &taskText) const
{
    const QString normalized = taskText.toCaseFolded().simplified();
    QVariantList recommendations;

    if (!normalized.isEmpty()) {
        for (const Rule &rule : rules) {
            const QRegularExpression expression(
                QString::fromLatin1(rule.patterns),
                QRegularExpression::CaseInsensitiveOption);
            if (!expression.match(normalized).hasMatch())
                continue;

            QVariantMap item;
            item.insert(QStringLiteral("templateId"), QString::fromLatin1(rule.id));
            item.insert(QStringLiteral("label"), QString::fromUtf8(rule.label));
            item.insert(QStringLiteral("reason"), QString::fromUtf8(rule.reason));
            item.insert(QStringLiteral("matchLevel"), QStringLiteral("LEXICAL"));
            recommendations.append(item);
        }
    }

    const bool needsClarification = recommendations.isEmpty();
    return {
        {QStringLiteral("recommendations"), recommendations},
        {QStringLiteral("needsClarification"), needsClarification},
        {QStringLiteral("clarificationPrompt"),
         needsClarification
             ? QStringLiteral("Уточните цель миссии: что требуется обследовать, измерить, обнаружить или доставить?")
             : QString()},
        {QStringLiteral("method"), QStringLiteral("explainable-lexical-baseline")},
        {QStringLiteral("operatorConfirmationRequired"), true},
        {QStringLiteral("flightReleaseEligible"), false}
    };
}

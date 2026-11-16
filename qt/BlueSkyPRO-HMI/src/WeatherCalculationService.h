#pragma once

#include <QFutureWatcher>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariantMap>
#include <QVariantList>
#include <optional>
#include <string>

#include "04_SOFTWARE/PLANNING/model/route_model.hpp"
#include "04_SOFTWARE/PLANNING/model/flight_safety_constraints.hpp"
#include "04_SOFTWARE/PLANNING/notam_prohibited_zone_validator.hpp"
#include "04_SOFTWARE/PLANNING/wind_performance_trajectory.hpp"
#include "04_SOFTWARE/PLANNING/orchestrator/algorithm_orchestrator.hpp"
#include "04_SOFTWARE/WEATHER/weather_trajectory_calculator.hpp"

struct WeatherCandidateEvidenceBundle
{
    // Preserve the exact typed decision and source objects used by the worker.
    // Lifecycle finalization must consume these values, never rebuild evidence
    // from the presentation QVariantMap.
    bluesky::planning::OrchestratorDecision planning_decision;
    bluesky::planning::NotamSnapshot notam_snapshot;
    bluesky::weather::WeatherTrajectoryEvaluation weather_evaluation;
    bluesky::planning::TrajectoryPerformanceInput performance_input;
    std::string vehicle_configuration_id;

    std::string selected_candidate_id;
    bluesky::planning::Route selected_route;
    bluesky::planning::FlightSafetyValidationResult flight_safety_validation;
    bluesky::planning::NotamValidationResult notam_validation;
    bluesky::planning::TrajectoryResult weather_trajectory;
    std::string weather_snapshot_id;
    std::string weather_snapshot_version;
};

struct WeatherCalculationWorkerResult
{
    QVariantMap presentation;
    std::optional<WeatherCandidateEvidenceBundle> selected_candidate_evidence;
};

class WeatherCalculationService final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString state READ state NOTIFY stateChanged)
    Q_PROPERTY(QString lastError READ lastError NOTIFY lastErrorChanged)
    Q_PROPERTY(QVariantMap result READ result NOTIFY resultChanged)
    Q_PROPERTY(bool decoderAvailable READ decoderAvailable CONSTANT)

public:
    explicit WeatherCalculationService(QObject* parent = nullptr);

    QString state() const { return m_state; }
    QString lastError() const { return m_lastError; }
    QVariantMap result() const { return m_result; }
    bool decoderAvailable() const;
    const std::optional<WeatherCandidateEvidenceBundle>& selectedCandidateEvidence() const { return m_selectedCandidateEvidence; }

    Q_INVOKABLE bool evaluateRoute(
        const QVariantList& forecastProducts,
        const QVariantMap& routeSnapshot,
        const QString& missionModelJson,
        const QString& missionStartUtc,
        const QVariantMap& validatedPerformanceProfile);

    // Weather-only re-ranking of a complete, already validated candidate set.
    // Every candidate must carry nominal metrics and a matching flight-safety attestation.
    Q_INVOKABLE bool evaluateCandidates(
        const QVariantList& forecastProducts,
        const QVariantList& candidateSnapshots,
        const QString& missionModelJson,
        const QString& missionStartUtc,
        const QVariantMap& validatedPerformanceProfile,
        const QStringList& objectivePriorities,
        const QVariantMap& normalizedNotamSnapshot);

signals:
    void stateChanged();
    void lastErrorChanged();
    void resultChanged();
    void evaluationFinished(const QVariantMap& result);
    void selectedCandidateEvidenceChanged();

private:
    void setState(const QString& state);
    void setError(const QString& error);

    QString m_state{QStringLiteral("IDLE")};
    QString m_lastError;
    QVariantMap m_result;
    QFutureWatcher<WeatherCalculationWorkerResult> m_watcher;
    std::optional<WeatherCandidateEvidenceBundle> m_selectedCandidateEvidence;
};

#include "flight/FlightData.h"
#include <cmath>
#include <algorithm>

namespace flight
{

    // ======================= Helpers internos  =========================
    static constexpr float EPS = 1e-4f; // tolerancias numéricas
    static constexpr float MPS_TO_KT = 1.94384f;
    static constexpr float MPS_TO_FTPM = 196.8504f;
    static constexpr float M_TO_FT = 3.28084f;

    static inline bool isFiniteVec(const glm::vec3 &v)
    {
        return std::isfinite(v.x) && std::isfinite(v.y) && std::isfinite(v.z);
    }

    // a en [0,360)
    static inline float normAngle360(float a)
    {
        a = std::fmod(a, 360.0f);
        if (a < 0.0f)
            a += 360.0f;
        return a;
    }

    // delta (a->b) corto en [-180,180)
    static inline float shortestDelta(float a, float b)
    {
        return std::fmod(b - a + 540.0f, 360.0f) - 180.0f;
    }

    // conserva prev si now es NaN/inf
    static inline float keepSane(float prev, float now)
    {
        return std::isfinite(now) ? now : prev;
    }

    // Ortonormaliza base (f,u) con fallback robusto; devuelve {f,r,u}
    static inline void makeCameraBasis(const glm::vec3 &frontIn,
                                       const glm::vec3 &upIn,
                                       const glm::vec3 &lastF,
                                       const glm::vec3 &lastU,
                                       glm::vec3 &f, glm::vec3 &r, glm::vec3 &u)
    {
        // front saneado
        glm::vec3 f0 = (isFiniteVec(frontIn) && glm::dot(frontIn, frontIn) > EPS * EPS)
                           ? frontIn
                           : (glm::dot(lastF, lastF) > EPS * EPS ? lastF : glm::vec3(0, 0, -1));
        f = glm::normalize(f0);

        // up de entrada saneado
        glm::vec3 u0 = (isFiniteVec(upIn) && glm::dot(upIn, upIn) > EPS * EPS)
                           ? upIn
                           : (glm::dot(lastU, lastU) > EPS * EPS ? lastU : glm::vec3(0, 1, 0));

        // right = f × u0 (puede degenerar si ~paralelos)
        r = glm::cross(f, u0);
        if (glm::dot(r, r) < EPS * EPS)
        {
            // elegir un “any” no paralelo a f (si f no es casi vertical → usa Y; si no, usa X)
            const glm::vec3 any = (std::abs(f.y) < 0.9f) ? glm::vec3(0, 1, 0) : glm::vec3(1, 0, 0);
            r = glm::cross(f, any);
        }
        r = glm::normalize(r);

        // u ortogonal y normalizado
        u = glm::normalize(glm::cross(r, f));
    }

    // ============================== Implementación ==============================
    void FlightData::updateFromCamera(const glm::vec3 &frontIn,
                                      const glm::vec3 &upIn,
                                      const glm::vec3 &pos,
                                      float deltaTime)
    {
        if (deltaTime <= 0.0f)
            return;

        // 1) Base de cámara robusta
        glm::vec3 f, r, u;
        makeCameraBasis(frontIn, upIn, cameraFront, cameraUp, f, r, u);
        cameraFront = f;
        cameraRight = r;
        cameraUp = u; // expone base saneada

        // 2) Actitud y rumbo desde base
        //    Pitch: componente vertical de "hacia dónde miro".
        const float newPitch = glm::degrees(std::asin(glm::clamp(f.y, -1.0f, 1.0f)));

        //    Heading: azimut en XZ. 0° = -Z (norte). atan2(x, -z).
        float newHeading;
        const float horizLen2 = f.x * f.x + f.z * f.z;
        if (horizLen2 >= EPS * EPS)
        {
            newHeading = glm::degrees(std::atan2(f.x, -f.z));
            newHeading = normAngle360(newHeading);
        }
        else
        {
            newHeading = heading; // mirando casi vertical → mantener
        }

        //    Roll: giro alrededor de f; derecha positivo. Compara up real vs up de referencia.
        float newRoll = roll;
        const glm::vec3 worldUp(0, 1, 0);
        glm::vec3 refR = glm::cross(f, worldUp);
        const float refRL2 = glm::dot(refR, refR);
        if (refRL2 >= EPS * EPS)
        {
            refR = glm::normalize(refR);
            const glm::vec3 refU = glm::normalize(glm::cross(refR, f)); // up “sin alabeo”
            const float s = glm::dot(f, glm::cross(u, refU));           // seno firmado alrededor de f
            const float c = glm::dot(u, refU);                          // coseno
            newRoll = glm::degrees(std::atan2(s, c));                   // (−180,180]
        }
        // Nota: si f≈worldUp, el alabeo no está bien definido → se preserva el valor anterior.

        // 3) Suavizado exponencial (evita jitter) + shortest-arc para ángulos
        const float dtClamped = glm::min(deltaTime, 0.25f);               // cap anti-pause
        const float alphaAtt = 1.0f - std::exp(-dtClamped / tauAttitude); // [0..1]

        const float prevPitch = pitch;
        const float prevRoll = roll;
        const float prevHeading = heading;
        const float prevYaw = yaw;

        pitch += shortestDelta(pitch, newPitch) * alphaAtt;
        roll += shortestDelta(roll, newRoll) * alphaAtt;
        heading += shortestDelta(heading, newHeading) * alphaAtt;

        // clamps/normalizaciones (post-suavizado)
        pitch = glm::clamp(pitch, -90.0f, 90.0f);
        roll = glm::clamp(roll, -180.0f, 180.0f);
        heading = normAngle360(heading);
        yaw = heading; // sin derrape

        // blindaje anti-NaN
        pitch = keepSane(prevPitch, pitch);
        roll = keepSane(prevRoll, roll);
        heading = keepSane(prevHeading, heading);
        yaw = keepSane(prevYaw, yaw);

        // 4) Telemetría de posición/velocidad
        const glm::vec3 dP = pos - position;
        const float disp = glm::length(dP);

        if (disp / deltaTime < maxPlausibleSpeed)
        {
            const glm::vec3 velMeasured = dP / deltaTime;
            const float alphaVel = 1.0f - std::exp(-dtClamped / tauVelocity);
            velocity += (velMeasured - velocity) * alphaVel;
        } // si no: ignorar spike (anti-teleport)

        position = pos;

        // 5) Derivados para instrumentos
        const float speed_mps = glm::length(velocity);
        airspeed = speed_mps * MPS_TO_KT; // “groundspeed” si no modelamos viento
        verticalSpeed = velocity.y * MPS_TO_FTPM;

        // 6) Altitud “QFE”: cero a ~altura de ojos (1.8 m). Fácil de visualizar en el HUD.
        static constexpr float EYE_LEVEL_Y_METERS = 1.8f;
        altitude = (position.y - EYE_LEVEL_Y_METERS) * M_TO_FT;

        // 7) Saneamiento final para instrumentos (HUD-ready)
        pitch = glm::clamp(pitch, -90.0f, 90.0f);
        roll = glm::clamp(roll, -180.0f, 180.0f);
        airspeed = glm::max(0.0f, airspeed);
        verticalSpeed = glm::clamp(verticalSpeed, -6000.0f, 6000.0f);
    }

    void FlightData::simulatePhysics(float /*deltaTime*/)
    {
        // TODO No implementado: en modo telemetría sólo se actualiza desde cámara
    }

    float FlightData::normalizeAngle(float angle) { return normAngle360(angle); }

} // namespace flight
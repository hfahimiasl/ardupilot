#include <AP_HAL/AP_HAL.h>
#include "AC_PrecLand_SITL.h"

#if AC_PRECLAND_SITL_ENABLED

#include "AP_AHRS/AP_AHRS.h"
// init - perform initialisation of this backend
void AC_PrecLand_SITL::init()
{
    _sitl = AP::sitl();
}

// update - give chance to driver to get updates from sensor
void AC_PrecLand_SITL::update()
{
    _state.healthy = _sitl->precland_sim.healthy();

    if (_state.healthy && _sitl->precland_sim.last_update_ms() != _los_meas.time_ms) {
        const Vector3d position = _sitl->precland_sim.get_target_position();
        const Matrix3d body_to_ned = AP::ahrs().get_rotation_body_to_ned().todouble();
        _los_meas.vec_unit =  body_to_ned.mul_transpose(-position).tofloat();
        _distance_to_target = _sitl->precland_sim.option_enabled(SITL::SIM_Precland::Option::ENABLE_TARGET_DISTANCE) ? _los_meas.vec_unit.length() : 0.0f;
        _los_meas.vec_unit /= _los_meas.vec_unit.length();
        _los_meas.frame = AC_PrecLand::VectorFrame::BODY_FRD;

        if (_frontend._orient != Rotation::ROTATION_PITCH_270) {
            // rotate body frame vector based on orientation
            // this is done to have homogeneity among backends
            // frontend rotates it back to get correct body frame vector
            _los_meas.vec_unit.rotate_inverse(_frontend._orient);
            _los_meas.vec_unit.rotate_inverse(ROTATION_PITCH_90);
        }

        _los_meas.valid = true;
        _los_meas.time_ms = _sitl->precland_sim.last_update_ms();
    } else {
        _los_meas.valid = false;
    }

    _los_meas.valid = _los_meas.valid && AP_HAL::millis() - _los_meas.time_ms <= 1000;
}

#endif  // AC_PRECLAND_SITL_ENABLED

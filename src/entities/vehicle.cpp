#include <entities/vehicle.h>
#include <entities/pkg.h>

#include <gamedata/bnd_file.h>

#include <r2/engine.h>
#include <r2/systems/cascade_functions.h>
#include <r2/utilities/interpolator.hpp>
using namespace r2;

#include <BulletCollision/CollisionShapes/btShapeHull.h>
#include <glm/gtc/type_ptr.hpp>

/* Notes:
	Breakable parts pivot around the X axis
	Wheels spin around the X axis and pivot around the Y axis
	Seems like everything that pivots does so around the X axis
	Shocks probably compress in the Z axis
 */

namespace sr2 {
	vehicle_camera::vehicle_camera(const mstring& name) : scene_entity(name) {
		angles.x = 2.0f * 3.141592653f;
		angles.y = angles.x;
	}

	vehicle_camera::~vehicle_camera() {
	}

	void vehicle_camera::onInitialize() {
		setUpdateFrequency(-1.0f);
		transform_sys::get()->addComponentTo(this);
		camera_sys::get()->addComponentTo(this);
		camera->activate();
		vec2f screen = r2engine::get()->window()->get_size();
		camera->projection = glm::perspective(glm::radians(90.0f), screen.x / screen.y, 0.1f, 4000.0f);
	}

	void vehicle_camera::onUpdate(f32 frameDt, f32 updateDt) {
		auto interpolateAngle = [](f32 fromAngle, f32 toAngle, f32 t) {
			static const f32 TWO_PI = 2.0f * 3.141592653f;
			static const f32 PI = 3.141592653f;

			fromAngle = fmodf((fromAngle + TWO_PI), TWO_PI);
			toAngle = fmodf((toAngle + TWO_PI), TWO_PI);

			f32 diff = fabsf(fromAngle - toAngle);
			if (diff < PI) {
				return fromAngle + ((toAngle - fromAngle) * t);
			} else {
				if (fromAngle > toAngle) {
					fromAngle = fromAngle - TWO_PI;
					return fromAngle + ((toAngle - fromAngle) * t);
				}
				else if (toAngle > fromAngle) {
					toAngle = toAngle - TWO_PI;
					return fromAngle + ((toAngle - fromAngle) * t);
				}
			}
		};

		vec2f screen = r2engine::get()->window()->get_size();
		camera->projection = glm::perspective(glm::radians(90.0f), screen.x / screen.y, 0.1f, 4000.0f);

		mat4f vt = tracking->transform->transform;
		vec3f target = vt[3] + vec4f(0, 2, 0, 0);
		vec3f back = vt[2] * -1.0f;
		f32 t_angle = -atan2f(back.z, back.x) + (3.141592653f * 0.5f);
		if (t_angle < 0.0f) { t_angle += 2.0f * 3.141592653f; }
		if (angles.x < 0.0f) { angles.x += 2.0f * 3.141592653f; }

		f32 diff = t_angle - angles.x;
		if (fabsf(diff) > 0.01) {
			angles.x = interpolateAngle(angles.x, t_angle, 5.9f * updateDt);
		} else angles.x = t_angle;


		f32 v_angle = atanf(vt[2].y);
		if (v_angle < 0.0f) { v_angle += 2.0f * 3.141592653f; }
		if (angles.y < 0.0f) { angles.y += 2.0f * 3.141592653f; }

		diff = v_angle - angles.y;
		if (fabsf(diff) > 0.01) {
			angles.y = interpolateAngle(angles.y, v_angle, 5.9f * updateDt);
		} else angles.y = v_angle;

		vec3f eye = vec3f(0, 3, -9);
		eye = glm::rotate(mat4f(1.0f), angles.y, vec3f(1, 0, 0)) * vec4f(eye, 1.0f);
		eye = glm::rotate(mat4f(1.0f), angles.x, vec3f(0, 1, 0)) * vec4f(eye, 1.0f);
		mat4f t = glm::lookAt(target + eye, target, vec3f(0, 1, 0));

		transform->transform = t;
	}

	// IMPORTANT NOTE: This function assumes that all meshes associated
	// with the entity (and its children) use vertex formats that specify
	// the position components first, and in x-y-z order
	// also, returns x = radius, y = thickness
	vec2f calculateWheelDimensions(scene_entity* entity) {
		f32 max_y = 0.0f;
		f32 min_x = FLT_MAX;
		f32 max_x = FLT_MIN;
		if (entity->mesh) {
			render_node* node = entity->mesh->get_node();
			u8* vertices = (u8*)node->vertex_data();
			size_t stride = node->vertices().buffer->format()->size();
			size_t vcount = node->vertex_count();
			for (size_t i = 0;i < vcount;i++) {
				size_t off = i * stride;
				vec3f* vert = (vec3f*)(vertices + off);
				if (vert->y > max_y) max_y = vert->y;
				if (vert->x < min_x) min_x = vert->x;
				if (vert->x > max_x) max_x = vert->x;
			}
		}

		return vec2f(max_y, max_x - min_x);
	}

	btTransform glmToBt(const mat4f& transform) {
		btTransform out;
		mat4f trns = transform;
		trns[0] = glm::normalize(trns[0]);
		trns[1] = glm::normalize(trns[1]);
		trns[2] = glm::normalize(trns[2]);
		out.setFromOpenGLMatrix(glm::value_ptr(trns));
		return out;
	}

	btVector3 glmToBt(const vec3f& v) {
		return btVector3(v.x, v.y, v.z);
	}

	// IMPORTANT NOTE: This function assumes that all meshes associated
	// with the entity (and its children) use vertex formats that specify
	// the position components first, and in x-y-z order
	btConvexHullShape* build_hull(scene_entity* entity, const vec3f& offset = vec3f(0.0f, 0.0f, 0.0f)) {
		btConvexHullShape initialHull;

		if (entity->mesh) {
			render_node* node = entity->mesh->get_node();
			u8* vertices = (u8*)node->vertex_data();
			size_t stride = node->vertices().buffer->format()->size();
			size_t vcount = node->vertex_count();
			for (size_t i = 0;i < vcount;i++) {
				size_t off = i * stride;
				vec3f* vert = (vec3f*)(vertices + off);
				initialHull.addPoint(glmToBt(*vert + offset));
			}
		}

		entity->for_each_child([&initialHull, offset](scene_entity* e) {
			if (e->mesh) {
				render_node* node = e->mesh->get_node();
				u8* vertices = (u8*)node->vertex_data();
				size_t stride = node->vertices().buffer->format()->size();
				size_t vcount = node->vertex_count();
				for (size_t i = 0;i < vcount;i++) {
					size_t off = i * stride;
					vec3f* vert = (vec3f*)(vertices + off);
					initialHull.addPoint(glmToBt(*vert + offset));
				}
			}
		});

		initialHull.setMargin(0.0f);
		btShapeHull* hull = new btShapeHull(&initialHull);
		hull->buildHull(0.0f);
		btConvexHullShape* shape = new btConvexHullShape(
			(f32*)hull->getVertexPointer(),
			hull->numVertices(),
			sizeof(btVector3)
		);
		delete hull;
		return shape;
	}

	btConvexHullShape* build_hull(bnd_file* bounds, const vec3f& offset = vec3f(0.0f, 0.0f, 0.0f)) {
		btConvexHullShape initialHull;
		for (u32 i = 0;i < bounds->vertices.size();i++) {
			initialHull.addPoint(glmToBt(bounds->vertices[i].position + offset));
		}

		initialHull.setMargin(0.0f);
		btShapeHull* hull = new btShapeHull(&initialHull);
		hull->buildHull(0.0f);
		btConvexHullShape* shape = new btConvexHullShape(
			(f32*)hull->getVertexPointer(),
			hull->numVertices(),
			sizeof(btVector3)
		);
		delete hull;
		return shape;
	}

	// todo: make this make more sense
	vehicle_entity* gvehicle = nullptr;
	void physicsTickCallback(btDynamicsWorld* world, btScalar timeStep) {
		if (!gvehicle) return;

		auto b = gvehicle->physics->rigidBody();
		btVector3 lv = b->getLinearVelocity();
		btVector3 av = b->getAngularVelocity();

		//drag = damping
		//down = force applied when in air (along y axis)

		vec3f ad = vec3f(1.0f, 1.0f, 1.0f) - (gvehicle->props.sim.aero.ang_c_damp * (1.0f / 5.0f));
		ad = vec3f(powf(ad.x, timeStep), powf(ad.y, timeStep), powf(ad.z, timeStep));
		b->setAngularVelocity(av * glmToBt(ad));

		
		btVector3 down = b->getWorldTransform().getBasis() * btVector3(0, lv.length() * -gvehicle->props.sim.aero.down, 0);
		down /= gvehicle->props.sim.mass;
		lv += down * timeStep;
		b->setLinearVelocity(lv);
	}

	vehicle_entity::vehicle_entity(const mstring& name) : scene_entity(name) {
		memset(wheels, 0, sizeof(wheels));
		memset(arms, 0, sizeof(arms));
		memset(shocks, 0, sizeof(shocks));
		memset(shafts, 0, sizeof(shafts));
		memset(sirens, 0, sizeof(sirens));
		memset(exhausts, 0, sizeof(exhausts));
		memset(axles, 0, sizeof(axles));
		memset(breakables, 0, sizeof(breakables));
		memset(variants, 0, sizeof(variants));
		memset(&state, 0, sizeof(vehicle_state));
		body = nullptr;
		engine = nullptr;
		oil = nullptr;
		shadow = nullptr;
		driver = nullptr;
		rayCaster = nullptr;
		vehicle = nullptr;
		bounds = nullptr;

		time = 0.0f;

		gvehicle = this;

		state.hp_to_engine_force = 8.0f;
		state.trans_gear_ratio = 0.5f;
		state.rear_gear_ratio = 0.5f;

		engine_sounds = nullptr;
	}

	vehicle_entity::~vehicle_entity() {
	}

	void vehicle_entity::willBeDestroyed() {
		props.destroy();
		if (props.sound.engine_sample_count > 0) {
			for (u8 i = 0;i < props.sound.engine_sample_count;i++) {
				delete engine_sounds[i];
			}
			delete [] engine_sounds;
		}
		engine_sounds = nullptr;
	}

	void vehicle_entity::onInitialize() {
		transform_sys* tsys = transform_sys::get();
		physics_sys* psys = physics_sys::get();
		tsys->addComponentTo(this);

		// initialize transforms
		for_each_child([tsys](scene_entity* e) {
			tsys->addComponentTo(e);
			if (e->has_children()) {
				e->for_each_child([tsys, e](scene_entity* gc) {
					e->transform->transform = ((pkg_entity*)gc)->trns;
					tsys->addComponentTo(gc);
					gc->transform->transform = mat4f(1.0f);
				});
			} else e->transform->transform = ((pkg_entity*)e)->trns;
		});

		r2engine::get()->remove_child(this);
		if (!body) return;
		start_periodic_updates();
		setUpdateFrequency(-1.0f);

		auto& pstate = psys->physState();
		pstate.enable();
		pstate->world->setInternalTickCallback(physicsTickCallback);

		psys->addComponentTo(this);
		physics->set_mass(props.sim.mass);
		physics->set_shape(bounds ? build_hull(bounds, props.sim.model_offset * 0.35f) : build_hull(body));
		physics->rigidBody()->setActivationState(DISABLE_DEACTIVATION);
		physics->rigidBody()->setDamping(1.0f - props.sim.aero.drag, 0.0f);

		btRaycastVehicle::btVehicleTuning tune;

		rayCaster = new btDefaultVehicleRaycaster(pstate->world);
		vehicle = new btRaycastVehicle(tune, physics->rigidBody(), rayCaster);
		vehicle->setCoordinateSystem(0, 1, 2);
		
		for (u8 i = 0;i < 4;i++) {
			if (!wheels[i]) continue;
			bool isFrontWheel = i <= 1;

			f32 sus_min = isFrontWheel ? props.sim.front_wheels.suspension_limit : props.sim.rear_wheels.suspension_limit;
			f32 sus_max = isFrontWheel ? props.sim.front_wheels.suspension_extent : props.sim.rear_wheels.suspension_extent;
			f32 sus_fac = isFrontWheel ? props.sim.front_wheels.suspension_factor : props.sim.rear_wheels.suspension_factor;
			f32 sus_dmp = isFrontWheel ? props.sim.front_wheels.suspension_damping : props.sim.rear_wheels.suspension_damping;
			f32 f_slip = isFrontWheel ? props.sim.front_wheels.optimum_slip_percent : props.sim.rear_wheels.optimum_slip_percent;

			vec2f dims = calculateWheelDimensions(wheels[i]);
			f32 wheelRadius = dims.x;
			f32 wheelWidth = dims.y;
			mat4f transform = wheels[i]->transform->transform;
			vec3f position = transform[3];
			btWheelInfo& info = vehicle->addWheel(
				glmToBt(position), 
				btVector3(0, -1, 0),
				btVector3(-1, 0, 0),
				sus_max,
				wheelRadius,
				tune,
				isFrontWheel
			);
			info.m_maxSuspensionTravelCm = sus_max * 100.0f;
			info.m_suspensionStiffness = sus_fac * 11.0f;
			info.m_wheelsDampingCompression = sus_dmp * 5.0f;
			info.m_maxSuspensionForce = props.sim.mass * 295.0f;
			info.m_frictionSlip = f_slip * 33.0f;
		}

		btTransform com;
		com.setIdentity();
		com.setOrigin(glmToBt(props.sim.center_of_mass));
		physics->rigidBody()->setCenterOfMassTransform(com);

		pstate->world->addAction(vehicle);
		pstate.disable();
	}

	void vehicle_entity::onUpdate(f32 frameDt, f32 updateDt) {
		time += updateDt;

		state.on_ground = false;
		for (u8 i = 0,w = 0;i < 4;i++) {
			if (!wheels[i]) continue;
			vehicle->updateWheelTransform(w, true);
			auto& wi = vehicle->getWheelInfo(w);
			if (wi.m_raycastInfo.m_groundObject) {
				state.on_ground = true;
			}
			mat4f wt;
			wi.m_worldTransform.getOpenGLMatrix(glm::value_ptr(wt));
			wt = glm::inverse(transform->transform) * wt;

			wheels[i]->transform->transform = wt;

			w++;
		}

		if (r2engine::input()->joystick_count() > 0) {
			auto js = r2engine::input()->joystick(0);
			auto jState = js->getJoyStickState();
			vec2f lstick = vec2f(f32(jState.mAxes[1].abs) / 32767.0f, f32(jState.mAxes[0].abs) / 32767.0f);
			if (fabsf(lstick.x) < 0.1f) lstick.x = 0.0f;
			state.steer_angle = lstick.x * -props.sim.front_wheels.steering_limit;
			if (state.steer_angle < -props.sim.front_wheels.steering_limit) state.steer_angle = -props.sim.front_wheels.steering_limit;
			else if (state.steer_angle > props.sim.front_wheels.steering_limit) state.steer_angle = props.sim.front_wheels.steering_limit;
			if (wheels[0]) vehicle->setSteeringValue(state.steer_angle, 0);
			if (wheels[1]) vehicle->setSteeringValue(state.steer_angle, 1);

			if (jState.mButtons[8]) {
				// accelerate
				state.engine_force = props.sim.engine.max_horsepower * state.hp_to_engine_force;
				state.brake_force = 0.0f;
				state.reverse = false;
			} else if (jState.mButtons[10]) {
				// brake
				state.engine_force = 0.0f;
				state.brake_force = 100.0f;
			} else if (jState.mButtons[11]) {
				// reverse
				state.engine_force = props.sim.engine.max_horsepower * state.hp_to_engine_force;
				state.brake_force = 0.0f;
				state.reverse = true;
			} else {
				// coast
				state.engine_force = 0.0f;
				state.brake_force = 0.0f;
			}

			vehicle->applyEngineForce(state.engine_force * (state.reverse ? 1.0f : -1.0f), 2);
			vehicle->applyEngineForce(state.engine_force * (state.reverse ? 1.0f : -1.0f), 3);
			vehicle->setBrake(state.brake_force, 0);
			vehicle->setBrake(state.brake_force, 1);
			vehicle->setBrake(state.brake_force, 2);
			vehicle->setBrake(state.brake_force, 3);
		}

		// scale the shocks and shafts
		for (u8 i = 0;i < 4;i++) {
			if (!wheels[i] || !shocks[i]) continue;

			mat4f wheelTrans = wheels[i]->transform->transform;

			vec3f wheelNow = wheelTrans * vec4f(0, 0, 0, 1);
			vec3f wheelInitial = ((pkg_entity*)wheels[i])->trns * vec4f(0, 0, 0, 1);
			vec3f shockTop = shocks[i]->transform->transform * vec4f(0, 0, 0, 1);
			f32 scale = (wheelNow.y - shockTop.y) / (wheelInitial.y - shockTop.y);
			
			// normalize shock scale
			mat4f shockTransform = shocks[i]->transform->transform;
			shockTransform[0] = glm::normalize(shockTransform[0]);
			shockTransform[1] = glm::normalize(shockTransform[1]);
			shockTransform[2] = glm::normalize(shockTransform[2]);
			shocks[i]->transform->transform = glm::scale(shockTransform, vec3f(1.0f, 1.0f, scale));

			if (!shafts[i]) continue;

			auto shaftParts = shafts[i]->children();
			mat4f shaftPartTransforms[2] = {
				shaftParts[0]->transform->transform,
				shaftParts[1]->transform->transform
			};

			vec3f shaftTop = shaftPartTransforms[1] * vec4f(0, 0, 0, 1);
			scale = (wheelNow.y - shaftTop.y) / (wheelInitial.y - shaftTop.y);
			scale = 1.0f + (1.0f - scale);

			// normalize shaft scale
			mat4f shaftTransform = shaftPartTransforms[1];
			shaftTransform[0] = glm::normalize(shaftTransform[0]);
			shaftTransform[1] = glm::normalize(shaftTransform[1]);
			shaftTransform[2] = glm::normalize(shaftTransform[2]);
			shaftParts[1]->transform->transform = glm::scale(shaftTransform, vec3f(1.0f, scale, 1.0f));

			//todo: automatic bone weights for stupid brake disk assembly
		}

		for_each_child([](scene_entity* e) {
			if (e->physics) return;

			if (e->mesh) {
				e->mesh->set_instance_transform(e->transform->cascaded_property(&transform_component::transform, &cascade_mat4f));
			}

			if (e->has_children()) {
				e->for_each_child([e](scene_entity* gc) {
					gc->mesh->set_instance_transform(gc->transform->cascaded_property(&transform_component::transform, &cascade_mat4f));
				});
			}
		});

		update_engine_sound();
	}

	f32 CalcRPM(f32 mph, f32 tireDiameter, f32 rearGearRatio, f32 transmissionGearRatio) {
		return floorf(((mph * rearGearRatio * transmissionGearRatio * 336.0f) / tireDiameter) * 100.0f + 0.5f) / 100.0f;
	}

	void vehicle_entity::update_engine_sound() {
		auto& backWheel = vehicle->getWheelInfo(2);
		f32 wheelDegreesPerSec = (backWheel.m_deltaRotation / (1.0f / 60.0f)) * 57.2958f;
		f32 rotationsPerSec = wheelDegreesPerSec / 360.0f;
		f32 wheelCircumference = 2.0f * 3.14159f * backWheel.m_wheelsRadius;
		f32 approxSpeedMS = wheelCircumference * rotationsPerSec;
		f32 approxSpeedMPH = approxSpeedMS * 2.23694f;
		// 0mph = idle, (props.sim.transmission.high)mph = OptRPM?
		f32 deducedEngineRPM = (approxSpeedMPH / props.sim.transmission.high) * (props.sim.engine.optimum_rpm - props.sim.engine.idle_rpm);

		deducedEngineRPM = fabs(CalcRPM(approxSpeedMPH, backWheel.m_wheelsRadius * 2.0f, state.rear_gear_ratio, state.trans_gear_ratio));
		deducedEngineRPM += props.sim.engine.idle_rpm;

		state.rpm = deducedEngineRPM;
		float rpm = fabs(state.rpm) / props.sim.engine.max_rpm;
		if (rpm < 0.0f) rpm = 0.0f;
		if (rpm > 1.0f) rpm = 1.0f;
		for (u8 i = 0;i < props.sound.engine_sample_count;i++) {
			if (!engine_sounds[i]) continue;
			auto& sample = props.sound.engine_samples[i];
			audio_source* src = engine_sounds[i];

			bool useSrc = false;
			for (u8 p = 0;p < sample.pitch_shift_count;p++) {
				auto& shifter = sample.pitch_shifters[p];
				if (rpm >= shifter.start_rpm && rpm <= shifter.end_rpm) {
					f32 fac = (rpm - shifter.start_rpm) / (shifter.end_rpm - shifter.start_rpm);
					src->setPitch(shifter.start_pitch + ((shifter.end_pitch - shifter.start_pitch) * fac));
					useSrc = true;
				}
			}

			for (u8 v = 0;v < sample.volume_fade_count;v++) {
				auto& fader = sample.volume_faders[v];
				if (rpm >= fader.fade_in_start_rpm && rpm <= fader.fade_in_end_rpm) {
					f32 fac = (rpm - fader.fade_in_start_rpm) / (fader.fade_in_end_rpm - fader.fade_in_start_rpm);
					src->setGain(fader.start_volume + ((fader.end_volume - fader.start_volume) * fac));
					useSrc = true;
				} else if (rpm < fader.fade_in_start_rpm) src->setGain(fader.start_volume);

				if (rpm >= fader.fade_out_start_rpm && rpm <= fader.fade_out_end_rpm && fader.fade_out_end_rpm != fader.fade_out_start_rpm) {
					f32 fac = (rpm - fader.fade_out_start_rpm) / (fader.fade_out_end_rpm - fader.fade_out_start_rpm);
					src->setGain(fader.end_volume + ((fader.start_volume - fader.end_volume) * fac));
					useSrc = true;
				} else if (rpm >= fader.fade_out_end_rpm) src->setGain(fader.end_volume);
			}

			if (useSrc && !src->isPlaying()) src->play();
			else if (!useSrc && src->isPlaying()) src->pause();
		}
	}

	void vehicle_entity::move_to(const mat4f& transform) {
		btTransform bodyTrans = glmToBt(transform);
		physics->set_transform(transform);
		physics->rigidBody()->setLinearVelocity(btVector3(0, 0, 0));
		physics->rigidBody()->setAngularVelocity(btVector3(0, 0, 0));

		for_each_child([transform](scene_entity* e) {
			if (e->physics) {
				mat4f initial = e->has_children() ? ((pkg_entity*)e->children()[0])->trns : ((pkg_entity*)e)->trns;
				mat4f now = transform * initial;
				e->physics->set_transform(now);
				e->physics->rigidBody()->setLinearVelocity(btVector3(0, 0, 0));
				e->physics->rigidBody()->setAngularVelocity(btVector3(0, 0, 0));
			}
		});
	}
};
#include <gamedata/vehicle_props.h>
#include <gamedata/manager.h>

#include <entities/vehicle.h>

#include <r2/engine.h>
#include <r2/utilities/utils.h>
using namespace r2;

namespace sr2 {
	vehicle_props::vehicle_props() {
		memset(&damage, 0, sizeof(damage));
		memset(&sim, 0, sizeof(sim));
		memset(&gyro, 0, sizeof(gyro));
		memset(&stuck, 0, sizeof(stuck));
		memset(&camera, 0, sizeof(camera));
		memset(&physics, 0, sizeof(physics));
		memset(&sound, 0, sizeof(sound));

		// damage
		{
			damage.double_pivot = 1;
			damage.mirror_pivot = 1;
			damage.impact_threshold = 1500.0f;
			damage.max_damage = 190999.953125f;
			damage.med_damage = 124999.96875f;
			damage.regenerate_rate = 8000.0f;
			damage.smoke_offset = vec3f(0.199999f, 0.989994f, 0.959994f);
			damage.textel_damage_radius = 0.5f;
		}

		// sim
		{
			sim.aero.ang_c_damp = vec3f(1.499992f, 1.99998f, 1.499992f);
			sim.aero.ang_vel_2_damp = vec3f(2.099987f, 1.099999f, 2.099976f);
			sim.aero.down = 2.499998f;
			sim.aero.drag = 0.8996f;

			sim.front_axel.damp_coef = 0.499962f;
			sim.front_axel.torque_coef = 1.0f;

			sim.rear_axel.damp_coef = 0.499962f;
			sim.rear_axel.torque_coef = 1.0f;

			sim.drivetrain.brake_static_coef = 1.2f;

			sim.engine.boost_duration = 1.2f;
			sim.engine.boost_horsepower = 1000.0f;
			sim.engine.gcl = 0.2f;
			sim.engine.max_horsepower = 800.0f;
			sim.engine.idle_rpm = 750.0f;
			sim.engine.max_rpm = 9000.0f;
			sim.engine.optimum_rpm = 7500.0f;

			sim.fluid.buoyancy = 0.4f;
			sim.fluid.damping = 0.02f;

			sim.freetrain.angular_inertia = 10.0f;
			sim.freetrain.brake_static_coef = 1.2f;

			sim.transmission.downshift_bias_max = 0.29996f;
			sim.transmission.upshift_bias = 0.0f;
			sim.transmission.gear_change_time = 0.899971f;
			sim.transmission.high = 99.0f;
			sim.transmission.low = 20.0f;

			sim.rear_wheels.brake_coef = 2.0f;
			sim.rear_wheels.camber_limit = 0.0f;
			sim.rear_wheels.handbrake_coef = 0.25f;
			sim.rear_wheels.optimum_slip_percent = 0.25f;
			sim.rear_wheels.sliding_friction = 2.099982f;
			sim.rear_wheels.suspension_damping = 0.09996f;
			sim.rear_wheels.suspension_extent = 0.342996;
			sim.rear_wheels.suspension_factor = 0.999999f;
			sim.rear_wheels.suspension_limit = 0.449975f;
			sim.rear_wheels.lateral_damping = 0.499995f;
			sim.rear_wheels.lateral_disp_limit = 0.149979f;
			sim.rear_wheels.longitudinal_disp_limit = 0.149979f;
			sim.rear_wheels.longitudinal_drag = 0.019978f;

			sim.front_wheels.brake_coef = 0.399962f;
			sim.front_wheels.camber_limit = 0.49f;
			sim.front_wheels.optimum_slip_percent = 0.5f;
			sim.front_wheels.sliding_friction = 2.299997f;
			sim.front_wheels.steering_limit = 0.40999f;
			sim.front_wheels.steering_offset = 0.32996f;
			sim.front_wheels.suspension_damping = 0.09996f;
			sim.front_wheels.suspension_extent = 0.449975f;
			sim.front_wheels.suspension_factor = 0.999999f;
			sim.front_wheels.suspension_limit = 0.499964f;
			sim.front_wheels.lateral_damping = 0.499995f;
			sim.front_wheels.lateral_disp_limit = 0.149979f;
			sim.front_wheels.longitudinal_disp_limit = 0.149979f;
			sim.front_wheels.longitudinal_drag = 0.019978f;

			sim.inertia_box = vec3f(2.5f, 1.099994f, 3.5f);
			sim.model_offset = vec3f(0.0f, -1.199977f, 0.0f);
			sim.bound_elasticity = 0.1f;
			sim.center_of_mass = vec3f(0.0f, 0.15f, 0.2f);
			sim.mass = 2000.0f;
			sim.air_gravity = 2.0f;
			sim.sss_threshold = 50.0f;
			sim.sss_value = 0.549998f;
		}

		// gyro
		{
			gyro.drift = 0.299999f;
			gyro.drift_thrust = 0.699998f;
			gyro.pitch = 9.0f;
			gyro.reverse_180 = 1.0f;
			gyro.roll = 6.0f;
			gyro.spin_180 = 0.149996f;
			gyro.turn = 0.049995f;
			gyro.wheelie = 25.0f;
		}

		// stuck
		{
			stuck.motion_threshold = 3.0f;
			stuck.position_threshold = 2.5f;
			stuck.rotation = -0.6f;
			stuck.time_threshold = 0.299997f;
			stuck.translation = 0.109998f;
			stuck.turn = 1.349999f;
		}

		// camera default values come from default_*.cam*CS
		// camera
		{
			camera.far.blend_time = 1.199998f;
			camera.far.field_of_view = 70.0f;
			camera.far.near_plane = 0.5f;
			camera.far.far_plane = 460.0f;
			camera.far.app_rot = 45.0f;
			camera.far.app_x_rot = 8.0f;
			camera.far.app_xz_pos = 1.5f;
			camera.far.track_to = vec3f(0.0f, 2.0f, 0.000001f);
			camera.far.max_dist = 15.0f;
			camera.far.min_dist = 10.0f;
			camera.far.offset = vec3f(0.0f, 1.25f, 12.0f);
			camera.far.app_xz_pos_min = 1.5f;
			camera.far.app_xz_pos_max = 6.0f;
			camera.far.hill_min = -0.699998f;
			camera.far.hill_max = 1.349999f;
			camera.far.hill_lerp = 0.5f;

			camera.near.blend_time = 1.199998f;
			camera.near.field_of_view = 70.0f;
			camera.near.near_plane = 0.5f;
			camera.near.far_plane = 460.0f;
			camera.near.app_rot = 30.0f;
			camera.near.app_x_rot = 8.0f;
			camera.near.app_y_pos = 6.999992f;
			camera.near.app_xz_pos = 2.0f;
			camera.near.app_app = 0.5f;
			camera.near.app_rot_min = 0.009994f;
			camera.near.track_to = vec3f(0.0f, 2.5f, -0.1f);
			camera.near.max_dist = 9.0f;
			camera.near.min_dist = 6.0f;
			camera.near.offset = vec3f(0.0f, 1.0f, 6.999998f);
			camera.near.app_xz_pos_min = 2.0f;
			camera.near.app_xz_pos_max = 8.0f;
			camera.near.hill_min = -0.8f;
			camera.near.hill_max = 1.329997f;
			camera.near.hill_lerp = 0.249999f;

			camera.pov.far_plane = 460.0f;
			camera.pov.app_xz_pos = 12.0f;
			camera.pov.track_to = vec3f(0.0f, 2.0f, 0.0f);
			camera.pov.max_dist = 6.0f;
			camera.pov.min_dist = 4.0f;
			camera.pov.offset = vec3f(0.0f, 0.5f, 5.0f);
		}

		// sound
		{
			sound.engine_sample_count = 3;
			sound.engine_samples = new engine_sample[3];
			sound.engine_samples[0].file_name = "sbugidl";
			sound.engine_samples[0].volume_fade_count = 1;
			sound.engine_samples[0].volume_faders = new engine_sample::volume_fade();
			sound.engine_samples[0].volume_faders[0].start_volume = 0.1f;
			sound.engine_samples[0].volume_faders[0].end_volume = 0.94f;
			sound.engine_samples[0].volume_faders[0].fade_in_start_rpm = 0.0f;
			sound.engine_samples[0].volume_faders[0].fade_in_end_rpm = 0.0f;
			sound.engine_samples[0].volume_faders[0].fade_out_start_rpm = 2500.0f;
			sound.engine_samples[0].volume_faders[0].fade_out_end_rpm = 5500.0f;
			sound.engine_samples[0].pitch_shift_count = 1;
			sound.engine_samples[0].pitch_shifters = new engine_sample::pitch_shift();
			sound.engine_samples[0].pitch_shifters[0].start_pitch = 0.5f;
			sound.engine_samples[0].pitch_shifters[0].end_pitch = 1.25f;
			sound.engine_samples[0].pitch_shifters[0].start_rpm = 0.0f;
			sound.engine_samples[0].pitch_shifters[0].end_rpm = 9250.0f;

			sound.engine_samples[1].file_name = "sbugmed";
			sound.engine_samples[1].volume_fade_count = 1;
			sound.engine_samples[1].volume_faders = new engine_sample::volume_fade();
			sound.engine_samples[1].volume_faders[0].start_volume = 0.1f;
			sound.engine_samples[1].volume_faders[0].end_volume = 0.95f;
			sound.engine_samples[1].volume_faders[0].fade_in_start_rpm = 1000.0f;
			sound.engine_samples[1].volume_faders[0].fade_in_end_rpm = 2500.0f;
			sound.engine_samples[1].volume_faders[0].fade_out_start_rpm = 3571.0f;
			sound.engine_samples[1].volume_faders[0].fade_out_end_rpm = 5500.0f;
			sound.engine_samples[1].pitch_shift_count = 1;
			sound.engine_samples[1].pitch_shifters = new engine_sample::pitch_shift();
			sound.engine_samples[1].pitch_shifters[0].start_pitch = 0.0f;
			sound.engine_samples[1].pitch_shifters[0].end_pitch = 2.75f;
			sound.engine_samples[1].pitch_shifters[0].start_rpm = 0.0f;
			sound.engine_samples[1].pitch_shifters[0].end_rpm = 18000.0f;

			sound.engine_samples[2].file_name = "sbughig";
			sound.engine_samples[2].volume_fade_count = 1;
			sound.engine_samples[2].volume_faders = new engine_sample::volume_fade();
			sound.engine_samples[2].volume_faders[0].start_volume = 0.1f;
			sound.engine_samples[2].volume_faders[0].end_volume = 0.92f;
			sound.engine_samples[2].volume_faders[0].fade_in_start_rpm = 2000.0f;
			sound.engine_samples[2].volume_faders[0].fade_in_end_rpm = 3500.0f;
			sound.engine_samples[2].volume_faders[0].fade_out_start_rpm = 88888.0f;
			sound.engine_samples[2].volume_faders[0].fade_out_end_rpm = 99999.0f;
			sound.engine_samples[2].pitch_shift_count = 1;
			sound.engine_samples[2].pitch_shifters = new engine_sample::pitch_shift();
			sound.engine_samples[2].pitch_shifters[0].start_pitch = 0.2f;
			sound.engine_samples[2].pitch_shifters[0].end_pitch = 1.5f;
			sound.engine_samples[2].pitch_shifters[0].start_rpm = 0.0f;
			sound.engine_samples[2].pitch_shifters[0].end_rpm = 19000.0f;
		}
	}

	void vehicle_props::destroy() {
		if (sound.engine_sample_count > 0) {
			for (u8 i = 0;i < sound.engine_sample_count;i++) {
				delete [] sound.engine_samples[i].volume_faders;
				delete [] sound.engine_samples[i].pitch_shifters;
			}
			delete[] sound.engine_samples;
			sound.engine_sample_count = 0;
		}
	}

	bool read_props(data_container* file, nlohmann::json& j) {
		mlist<mstring> path;
		while (!file->at_end(1)) {
			mstring s;
			if (!file->read_line(s)) break;
			if (s.length() == 0) continue;

			mvector<mstring> comps = split(s, " \t\n");
			if (comps.size() == 0) continue;

			if (comps[0] == "type:") {
				if (comps[1] != "a") {
					r2Warn("Unknown vehicle props type: %s", comps[1].c_str());
				}
				j["type"] = comps[1];
				continue;
			}

			if (comps.size() > 1 && comps[1] == "{") { path.push_back(comps[0]); continue; }
			if (comps.size() == 1 && comps[0] == "}") { path.pop_back(); continue; }

			nlohmann::json* root = &j;
			for (mstring& p : path) root = &(*root)[p];
			nlohmann::json& propRoot = *root;

			if (comps.size() == 2) {
				if (comps[1].find_first_of('.') != mstring::npos) propRoot[comps[0]] = atof(comps[1].c_str());
				else propRoot[comps[0]] = atoi(comps[1].c_str());
			} else if (comps.size() == 3) {
				if (comps[1].find_first_of('.') != mstring::npos) propRoot[comps[0]]["x"] = atof(comps[1].c_str());
				else propRoot[comps[0]]["x"] = atoi(comps[1].c_str());

				if (comps[2].find_first_of('.') != mstring::npos) propRoot[comps[0]]["y"] = atof(comps[2].c_str());
				else propRoot[comps[0]]["y"] = atoi(comps[2].c_str());
			} else if (comps.size() == 4) {
				if (comps[1].find_first_of('.') != mstring::npos) propRoot[comps[0]]["x"] = atof(comps[1].c_str());
				else propRoot[comps[0]]["x"] = atoi(comps[1].c_str());

				if (comps[2].find_first_of('.') != mstring::npos) propRoot[comps[0]]["y"] = atof(comps[2].c_str());
				else propRoot[comps[0]]["y"] = atoi(comps[2].c_str());

				if (comps[2].find_first_of('.') != mstring::npos) propRoot[comps[0]]["z"] = atof(comps[3].c_str());
				else propRoot[comps[0]]["z"] = atoi(comps[3].c_str());
			} else if (comps.size() == 5) {
				if (comps[1].find_first_of('.') != mstring::npos) propRoot[comps[0]]["r"] = atof(comps[1].c_str());
				else propRoot[comps[0]]["r"] = atoi(comps[1].c_str());

				if (comps[2].find_first_of('.') != mstring::npos) propRoot[comps[0]]["g"] = atof(comps[2].c_str());
				else propRoot[comps[0]]["g"] = atoi(comps[2].c_str());

				if (comps[2].find_first_of('.') != mstring::npos) propRoot[comps[0]]["b"] = atof(comps[3].c_str());
				else propRoot[comps[0]]["b"] = atoi(comps[3].c_str());

				if (comps[2].find_first_of('.') != mstring::npos) propRoot[comps[0]]["a"] = atof(comps[4].c_str());
				else propRoot[comps[0]]["a"] = atoi(comps[4].c_str());
			}
		}

		return true;
	}

	bool read_engine_sound_csv(data_container* file, vehicle_props* props) {
		bool sample_name_next = false;
		bool column_header_next = false;
		bool is_fader = false;
		mvector<mstring> sample_files;
		mvector<mvector<vehicle_props::engine_sample::volume_fade>> sample_faders;
		mvector<mvector<vehicle_props::engine_sample::pitch_shift>> sample_shifters;

		while (!file->at_end(1)) {
			mstring s;
			if (!file->read_line(s)) break;
			if (s.length() == 0) continue;

			if (s.find("REV") != mstring::npos) {
				f32 rev = atof(s.substr(3).c_str());
				if (rev != 4.0f) {
					r2Warn("Unknown engine csv revision %f", rev);
				}
				continue;
			}

			if (s.find("Sample name") != mstring::npos) {
				sample_name_next = true;
				continue;
			}

			if (sample_name_next) {
				sample_files.push_back(s);
				sample_faders.push_back(mvector<vehicle_props::engine_sample::volume_fade>());
				sample_shifters.push_back(mvector<vehicle_props::engine_sample::pitch_shift>());
				sample_name_next = false;
				column_header_next = true;
				continue;
			}

			mvector<mstring> cols = split(s, ", \t");
			if (cols.size() == 0) continue;

			if (column_header_next) {
				column_header_next = false;
				if (cols[0] == "Min" && cols[1] == "volume") is_fader = true;
				else is_fader = false;
				continue;
			}

			if (cols[0] == "Min" && cols[1] == "Pitch") {
				is_fader = false;
				continue;
			}

			if (is_fader) {
				vehicle_props::engine_sample::volume_fade fader;
				fader.start_volume = atof(cols[0].c_str());
				fader.end_volume = atof(cols[1].c_str());
				fader.fade_in_start_rpm = atof(cols[2].c_str());
				fader.fade_in_end_rpm = atof(cols[3].c_str());
				fader.fade_out_start_rpm = atof(cols[4].c_str());
				fader.fade_out_end_rpm = atof(cols[5].c_str());
				sample_faders[sample_files.size() - 1].push_back(fader);
			} else {
				vehicle_props::engine_sample::pitch_shift shifter;
				shifter.start_pitch = atof(cols[0].c_str());
				shifter.end_pitch = atof(cols[1].c_str());
				shifter.start_rpm = atof(cols[2].c_str());
				shifter.end_rpm = atof(cols[3].c_str());
				sample_shifters[sample_files.size() - 1].push_back(shifter);
			}
		}

		if (props->sound.engine_sample_count > 0) {
			for (u8 i = 0;i < props->sound.engine_sample_count;i++) {
				delete [] props->sound.engine_samples[i].volume_faders;
				delete [] props->sound.engine_samples[i].pitch_shifters;
			}
			delete[] props->sound.engine_samples;
		}

		props->sound.engine_sample_count = sample_files.size();
		props->sound.engine_samples = new vehicle_props::engine_sample[sample_files.size()];
		for (u8 i = 0;i < sample_files.size();i++) {
			props->sound.engine_samples[i].file_name = sample_files[i];
			props->sound.engine_samples[i].volume_fade_count = sample_faders[i].size();
			props->sound.engine_samples[i].volume_faders = new vehicle_props::engine_sample::volume_fade[sample_faders[i].size()];
			memcpy(props->sound.engine_samples[i].volume_faders, sample_faders[i].data(), sizeof(vehicle_props::engine_sample::volume_fade) * sample_faders[i].size());
			props->sound.engine_samples[i].pitch_shift_count = sample_shifters[i].size();
			props->sound.engine_samples[i].pitch_shifters = new vehicle_props::engine_sample::pitch_shift[sample_shifters[i].size()];
			memcpy(props->sound.engine_samples[i].pitch_shifters, sample_shifters[i].data(), sizeof(vehicle_props::engine_sample::pitch_shift) * sample_shifters[i].size());
		}

		return true;
	}


	vehicle_props_loader::vehicle_props_loader() { }
	vehicle_props_loader::~vehicle_props_loader() { }

	bool vehicle_props_loader::load(gamedata_manager* gdm, vehicle_entity* vehicle, const r2::mstring& vehicleName) {
		auto process = [gdm, this](const mstring& file, const mstring& rootKey = "") {
			data_container* c = gdm->open(file, DM_TEXT);
			if (c) {
				bool result = read_props(c, rootKey.length() == 0 ? this->jsonProps : this->jsonProps[rootKey]);
				r2engine::files()->destroy(c);
				return result;
			}

			return true;
		};

		if (!process("tune/vehicle/" + vehicleName + ".vehCarSim")) return false;
		if (!process("tune/vehicle/" + vehicleName + ".vehGyro")) return false;
		if (!process("tune/vehicle/" + vehicleName + ".vehStuck")) return false;
		if (!process("tune/vehicle/" + vehicleName + ".vehCarDamage")) return false;
		if (!process("tune/vehicle/" + vehicleName + ".vehDriver")) return false;
		if (!process("tune/camera/" + vehicleName + "_FAR.camTrackCS", "cam_far")) return false;
		if (!process("tune/camera/" + vehicleName + "_NEAR.camTrackCS", "cam_near")) return false;
		if (!process("tune/camera/" + vehicleName + "_POV.camTrackCS", "cam_pov")) return false;
		if (!process("tune/camera/" + vehicleName + "_POV.camPovCS", "cam_pov_unk")) return false;

		auto children = vehicle->children();
		for (scene_entity* child : children) {
			mstring cname = child->name();
			if (cname == "BODY" || cname == "SHADOW") continue;

			if (cname.find("SIREN") != mstring::npos) {
				// don't know why...
				i32 idx = atoi(cname.substr(5).c_str());
				cname = format_string("SRN%d", idx + 1);
			}

			if (!process(format_string("tune/banger/%s_%s.dgBangerData", vehicleName.c_str(), cname.c_str()), cname)) return false;
		}

		data_container* eng_noise = gdm->open("aud/cardata/player/engines/" + vehicleName + "_engine.csv", DM_TEXT);
		if (!eng_noise) eng_noise = gdm->open("aud/cardata/player/engines/default_engine.csv", DM_TEXT);
		if (eng_noise) {
			if (!read_engine_sound_csv(eng_noise, &vehicle->props)) return false;
			r2engine::files()->destroy(eng_noise);
		}

		props(vehicle);
		return true;
	}

	void vehicle_props_loader::props(vehicle_entity* vehicle) {
		vehicle_props& p = vehicle->props;
		const u8 t_int = 0;
		const u8 t_float = 1;
		const u8 t_vec3 = 2;
		const u8 t_vec4 = 3;
		auto take = [this](const mstring& path, u8 type, void* dest) {
			auto pth = split(path, ".");
			nlohmann::json* jval = &this->jsonProps;
			for (mstring& key : pth) {
				if (jval->contains(key)) jval = &(*jval)[key];
				else return;
			}
			if (type == 0) {
				(*(i32*)dest) = *jval;
			} else if (type == 1) {
				(*(f32*)dest) = *jval;
			} else if (type == 2) {
				vec3f& v = (*(vec3f*)dest);
				if (jval->contains("x")) v.x = (*jval)["x"];
				if (jval->contains("y")) v.y = (*jval)["y"];
				if (jval->contains("z")) v.z = (*jval)["z"];
			} else if (type == 3) {
				vec4f& v = (*(vec4f*)dest);
				if (jval->contains("r")) v.x = (*jval)["r"];
				if (jval->contains("g")) v.y = (*jval)["g"];
				if (jval->contains("b")) v.z = (*jval)["b"];
				if (jval->contains("a")) v.w = (*jval)["a"];
			}
		};

		// damage
		{
			take("vehCarDamage.DoublePivot", t_int, &p.damage.double_pivot);
			take("vehCarDamage.MirrorPivot", t_int, &p.damage.mirror_pivot);
			take("vehCarDamage.ImpactThreshold", t_float, &p.damage.impact_threshold);
			take("vehCarDamage.MaxDamage", t_float, &p.damage.max_damage);
			take("vehCarDamage.MedDamage", t_float, &p.damage.med_damage);
			take("vehCarDamage.RegenerateRate", t_float, &p.damage.regenerate_rate);
			take("vehCarDamage.SmokeOffset", t_vec3, &p.damage.smoke_offset);
			take("vehCarDamage.TextelDamageRadius", t_float, &p.damage.textel_damage_radius);
		}

		// sim
		{
			take("vehCarSim.Aero.AngCDamp", t_vec3, &p.sim.aero.ang_c_damp);
			take("vehCarSim.Aero.AngVel2Damp", t_vec3, &p.sim.aero.ang_vel_2_damp);
			take("vehCarSim.Aero.Down", t_float, &p.sim.aero.down);
			take("vehCarSim.Aero.Drag", t_float, &p.sim.aero.drag);

			take("vehCarSim.AxelBack.DampCoef", t_float, &p.sim.rear_axel.damp_coef);
			take("vehCarSim.AxelBack.TorqueCoef", t_float, &p.sim.rear_axel.torque_coef);

			take("vehCarSim.AxelFront.DampCoef", t_float, &p.sim.front_axel.damp_coef);
			take("vehCarSim.AxelFront.TorqueCoef", t_float, &p.sim.front_axel.torque_coef);

			take("vehCarSim.Drivetrain.BrakeStaticCoef", t_float, &p.sim.drivetrain.brake_static_coef);

			take("vehCarSim.Engine.BoostDuration", t_float, &p.sim.engine.boost_duration);
			take("vehCarSim.Engine.BoostHP", t_float, &p.sim.engine.boost_horsepower);
			take("vehCarSim.Engine.GCL", t_float, &p.sim.engine.gcl);
			take("vehCarSim.Engine.MaxHorsePower", t_float, &p.sim.engine.max_horsepower);
			take("vehCarSim.Engine.MaxRPM", t_float, &p.sim.engine.max_rpm);
			take("vehCarSim.Engine.OptRPM", t_float, &p.sim.engine.optimum_rpm);

			take("vehCarSim.Fluid.Buoyancy", t_float, &p.sim.fluid.buoyancy);
			take("vehCarSim.Fluid.Damp", t_float, &p.sim.fluid.damping);

			take("vehCarSim.Freetrain.AngInertia", t_float, &p.sim.freetrain.angular_inertia);
			take("vehCarSim.Freetrain.BrakeStaticCoef", t_float, &p.sim.freetrain.brake_static_coef);
		
			take("vehCarSim.WheelFront.BrakeCoef", t_float, &p.sim.front_wheels.brake_coef);
			take("vehCarSim.WheelFront.CamberLimit", t_float, &p.sim.front_wheels.camber_limit);
			take("vehCarSim.WheelFront.HandbrakeCoef", t_float, &p.sim.front_wheels.handbrake_coef);
			take("vehCarSim.WheelFront.OptimumSlipPercent", t_float, &p.sim.front_wheels.optimum_slip_percent);
			take("vehCarSim.WheelFront.SlidingFric", t_float, &p.sim.front_wheels.sliding_friction);
			take("vehCarSim.WheelFront.SteeringLimit", t_float, &p.sim.front_wheels.steering_limit);
			take("vehCarSim.WheelFront.SteeringOffset", t_float, &p.sim.front_wheels.steering_offset);
			take("vehCarSim.WheelFront.SuspensionDampCoef", t_float, &p.sim.front_wheels.suspension_damping);
			take("vehCarSim.WheelFront.SuspensionExtent", t_float, &p.sim.front_wheels.suspension_extent);
			take("vehCarSim.WheelFront.SuspensionFactor", t_float, &p.sim.front_wheels.suspension_factor);
			take("vehCarSim.WheelFront.SuspensionLimit", t_float, &p.sim.front_wheels.suspension_limit);
			take("vehCarSim.WheelFront.TireDampCoefLat", t_float, &p.sim.front_wheels.lateral_damping);
			take("vehCarSim.WheelFront.TireDispLimitLat", t_float, &p.sim.front_wheels.lateral_disp_limit);
			take("vehCarSim.WheelFront.TireDispLimitLong", t_float, &p.sim.front_wheels.longitudinal_disp_limit);
			take("vehCarSim.WheelFront.TireDragCoefLong", t_float, &p.sim.front_wheels.longitudinal_drag);

			take("vehCarSim.WheelBack.BrakeCoef", t_float, &p.sim.rear_wheels.brake_coef);
			take("vehCarSim.WheelBack.CamberLimit", t_float, &p.sim.rear_wheels.camber_limit);
			take("vehCarSim.WheelBack.HandbrakeCoef", t_float, &p.sim.rear_wheels.handbrake_coef);
			take("vehCarSim.WheelBack.OptimumSlipPercent", t_float, &p.sim.rear_wheels.optimum_slip_percent);
			take("vehCarSim.WheelBack.SlidingFric", t_float, &p.sim.rear_wheels.sliding_friction);
			take("vehCarSim.WheelBack.SuspensionDampCoef", t_float, &p.sim.rear_wheels.suspension_damping);
			take("vehCarSim.WheelBack.SuspensionExtent", t_float, &p.sim.rear_wheels.suspension_extent);
			take("vehCarSim.WheelBack.SuspensionFactor", t_float, &p.sim.rear_wheels.suspension_factor);
			take("vehCarSim.WheelBack.SuspensionLimit", t_float, &p.sim.rear_wheels.suspension_limit);
			take("vehCarSim.WheelBack.TireDampCoefLat", t_float, &p.sim.rear_wheels.lateral_damping);
			take("vehCarSim.WheelBack.TireDispLimitLat", t_float, &p.sim.rear_wheels.lateral_disp_limit);
			take("vehCarSim.WheelBack.TireDispLimitLong", t_float, &p.sim.rear_wheels.longitudinal_disp_limit);
			take("vehCarSim.WheelBack.TireDragCoefLong", t_float, &p.sim.rear_wheels.longitudinal_drag);

			take("vehCarSim.ModelOffset", t_vec3, &p.sim.model_offset);
			take("vehCarSim.BoundElasticity", t_float, &p.sim.bound_elasticity);
			take("vehCarSim.SSSThreshold", t_float, &p.sim.sss_threshold);
			take("vehCarSim.SSSValue", t_float, &p.sim.sss_value);
			take("vehCarSim.Mass", t_float, &p.sim.mass);
			take("vehCarSim.AirGravity", t_float, &p.sim.air_gravity);
			take("vehCarSim.CenterOfMass", t_vec3, &p.sim.center_of_mass);
		}

		// gyro
		{
			take("vehGyro.Drift", t_float, &p.gyro.drift);
			take("vehGyro.DriftThrust", t_float, &p.gyro.drift_thrust);
			take("vehGyro.Pitch", t_float, &p.gyro.pitch);
			take("vehGyro.Reverse180", t_float, &p.gyro.reverse_180);
			take("vehGyro.Roll", t_float, &p.gyro.roll);
			take("vehGyro.Spin180", t_float, &p.gyro.spin_180);
			take("vehGyro.Turn", t_float, &p.gyro.turn);
			take("vehGyro.Wheelie", t_float, &p.gyro.wheelie);
		}

		// stuck
		{
			take("vehStuck.MoveThresh", t_float, &p.stuck.motion_threshold);
			take("vehStuck.PosThresh", t_float, &p.stuck.position_threshold);
			take("vehStuck.Rotation", t_float, &p.stuck.rotation);
			take("vehStuck.TimeThresh", t_float, &p.stuck.time_threshold);
			take("vehStuck.Translation", t_float, &p.stuck.translation);
			take("vehStuck.Turn", t_float, &p.stuck.turn);
		}

		auto take_phys = [this, take, t_int, t_float, t_vec3](const mstring& path, vehicle_props::physics_info* dest) {
			if (this->jsonProps.contains(path)) dest->exists = true;
			else return;

			take(path + ".dgBangerData.AudioId", t_int, &dest->audio_id);
			take(path + ".dgBangerData.Size", t_vec3, &dest->size);
			take(path + ".dgBangerData.CG", t_vec3, &dest->center_of_gravity);
			take(path + ".dgBangerData.NumGlows", t_int, &dest->number_of_glows);
			take(path + ".dgBangerData.Mass", t_float, &dest->mass);
			take(path + ".dgBangerData.Elasticity", t_float, &dest->elasticity);
			take(path + ".dgBangerData.Friction", t_float, &dest->friction);
			take(path + ".dgBangerData.ImpulseLimit2", t_float, &dest->impulse_limit_2);
			take(path + ".dgBangerData.SpinAxis", t_int, &dest->spin_axis);
			take(path + ".dgBangerData.Flash", t_int, &dest->flash);
			take(path + ".dgBangerData.NumParts", t_int, &dest->number_of_parts);
			take(path + ".dgBangerData.TexNumber", t_int, &dest->tex_number);
			take(path + ".dgBangerData.BillFlags", t_int, &dest->bill_flags);
			take(path + ".dgBangerData.YRadius", t_float, &dest->y_radius);
			take(path + ".dgBangerData.ColliderId", t_int, &dest->collider_id);
			take(path + ".dgBangerData.CollisionPrim", t_int, &dest->collision_primitive);
			take(path + ".dgBangerData.CollisionType", t_int, &dest->collision_type);

			take(path + ".dgBangerData.BirthRule.Position", t_vec3, &dest->birth_rule.position);
			take(path + ".dgBangerData.BirthRule.PositionVar", t_vec3, &dest->birth_rule.position_variation);
			take(path + ".dgBangerData.BirthRule.Velocity", t_vec3, &dest->birth_rule.velocity);
			take(path + ".dgBangerData.BirthRule.VelocityVar", t_vec3, &dest->birth_rule.velocity_variation);
			take(path + ".dgBangerData.BirthRule.Life", t_float, &dest->birth_rule.lifespan);
			take(path + ".dgBangerData.BirthRule.Mass", t_float, &dest->birth_rule.mass);
			take(path + ".dgBangerData.BirthRule.MassVar", t_float, &dest->birth_rule.mass_variation);
			take(path + ".dgBangerData.BirthRule.Radius", t_float, &dest->birth_rule.radius);
			take(path + ".dgBangerData.BirthRule.RadiusVar", t_float, &dest->birth_rule.radius_variation);
			take(path + ".dgBangerData.BirthRule.Drag", t_float, &dest->birth_rule.drag);
			take(path + ".dgBangerData.BirthRule.DragVar", t_float, &dest->birth_rule.drag_variation);
			take(path + ".dgBangerData.BirthRule.DRadius", t_float, &dest->birth_rule.d_radius);
			take(path + ".dgBangerData.BirthRule.DRadiusVar", t_float, &dest->birth_rule.d_radius_variation);
			take(path + ".dgBangerData.BirthRule.DAlpha", t_int, &dest->birth_rule.d_alpha);
			take(path + ".dgBangerData.BirthRule.DAlphaVar", t_int, &dest->birth_rule.d_alpha_variation);
			take(path + ".dgBangerData.BirthRule.DRotation", t_int, &dest->birth_rule.d_rotation);
			take(path + ".dgBangerData.BirthRule.DRotationVar", t_int, &dest->birth_rule.d_rotation_variation);
			take(path + ".dgBangerData.BirthRule.InitialBlast", t_int, &dest->birth_rule.initial_blast);
			take(path + ".dgBangerData.BirthRule.SpewRate", t_float, &dest->birth_rule.spawn_rate);
			take(path + ".dgBangerData.BirthRule.SpewTimeLimit", t_float, &dest->birth_rule.spawn_duration);
			take(path + ".dgBangerData.BirthRule.Gravity", t_float, &dest->birth_rule.gravity);
			take(path + ".dgBangerData.BirthRule.TexFrameStart", t_int, &dest->birth_rule.tex_frame_start);
			take(path + ".dgBangerData.BirthRule.TexFrameEnd", t_int, &dest->birth_rule.tex_frame_end);
			take(path + ".dgBangerData.BirthRule.BirthFlags", t_int, &dest->birth_rule.birth_flags);
			take(path + ".dgBangerData.BirthRule.Color", t_int, &dest->birth_rule.color_index);
		};

		for (u8 i = 0;i < 8;i++) {
			if (vehicle->breakables[i]) take_phys(format_string("BREAK%d", i), &p.physics.breakables[i]);
			if (vehicle->variants[i]) take_phys(format_string("VARIANT%d", i), &p.physics.variants[i]);

			if (i >= 4) continue;
			if (vehicle->wheels[i]) take_phys(format_string("WHL%d", i), &p.physics.wheels[i]);
			if (vehicle->arms[i]) take_phys(format_string("ARM%d", i), &p.physics.arms[i]);
			if (vehicle->shocks[i]) take_phys(format_string("SHOCK%d", i), &p.physics.shocks[i]);
			if (vehicle->shafts[i]) take_phys(format_string("SHAFT%d", i), &p.physics.shafts[i]);
			if (vehicle->sirens[i]) take_phys(format_string("SRN%d", i + 1), &p.physics.sirens[i]);

			if (i >= 2) continue;
			if (vehicle->exhausts[i]) take_phys(format_string("EXHAUST%d", i), &p.physics.exhausts[i]);
			if (vehicle->axles[i]) take_phys(format_string("AXLE%d", i), &p.physics.axles[i]);
		}

		if (vehicle->engine) take_phys("ENGINE", &p.physics.engine);
		if (vehicle->oil) take_phys("OIL", &p.physics.oil);
		if (vehicle->driver) take_phys("DRIVER", &p.physics.driver);
	}
};
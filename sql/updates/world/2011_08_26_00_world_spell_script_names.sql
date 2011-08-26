DELETE FROM `spell_script_names` WHERE `spell_id` IN (47496);
INSERT INTO `spell_script_names` (`spell_id`,`ScriptName`) VALUES
(47496,'spell_dk_ghoul_explode');

DELETE FROM `spell_script_names` WHERE `spell_id` IN (50365,50391);
INSERT INTO `spell_script_names` (`spell_id`,`ScriptName`) VALUES
(50365,'spell_dk_improved_blood_presence'),
(50391,'spell_dk_improved_unholy_presence');

DELETE FROM `spell_script_names` WHERE `ScriptName` IN('spell_gen_av_drekthar_presence','spell_ex_463');
INSERT INTO `spell_script_names` (`spell_id`,`ScriptName`) VALUES
(463,   'spell_ex_463'),
(45828, 'spell_gen_av_drekthar_presence'),
(45829, 'spell_gen_av_drekthar_presence'),
(45830, 'spell_gen_av_drekthar_presence'),
(45822, 'spell_gen_av_drekthar_presence'),
(45823, 'spell_gen_av_drekthar_presence'),
(45824, 'spell_gen_av_drekthar_presence'),
(45826, 'spell_gen_av_drekthar_presence');

DELETE FROM `spell_script_names` WHERE `spell_id`=48917;
INSERT INTO `spell_script_names` (`spell_id`,`ScriptName`) VALUES
(48917, 'spell_q10041_q10040_who_are_they');

DELETE FROM `spell_script_names` WHERE `spell_id` = 24778;
INSERT INTO `spell_script_names` (`spell_id`,`ScriptName`) VALUES
(24778, 'spell_dream_fog_sleep');

DELETE FROM `spell_script_names` WHERE `ScriptName`='spell_mark_of_nature';
INSERT INTO `spell_script_names` (`spell_id`,`ScriptName`) VALUES
(25042, 'spell_mark_of_nature');

DELETE FROM `spell_script_names` WHERE `spell_id`=29371;
INSERT INTO `spell_script_names` (`spell_id`,`ScriptName`)
VALUES
(29371,'spell_heigan_eruption');

DELETE FROM `spell_script_names` WHERE `spell_id`=48095;
INSERT INTO `spell_script_names` (`spell_id`,`ScriptName`)
VALUES
(48095,'spell_intense_cold');

DELETE FROM `spell_script_names` WHERE `spell_id` IN (63317,64021);
INSERT INTO `spell_script_names` (`spell_id`,`ScriptName`)
VALUES
(63317,'spell_razorscale_flame_breath'),
(64021,'spell_razorscale_flame_breath');

DELETE FROM `spell_script_names` WHERE `spell_id` IN (62038,62039);
INSERT INTO `spell_script_names` (`spell_id`,`ScriptName`)
VALUES
(62038,'spell_biting_cold'),
(62039,'spell_biting_cold_dot');

-- Change spell script name for Attuned To Nature Dose Reduction
UPDATE `spell_script_names` SET `ScriptName`='spell_freya_attuned_to_nature_dose_reduction' WHERE `ScriptName`='spell_attuned_to_nature_dose_reduction';

DELETE FROM `spell_script_names` WHERE `spell_id` IN (62519,65158,65160);
INSERT INTO `spell_script_names` VALUES
(62519, 'spell_freya_attuned_to_nature'),
(65158, 'spell_freya_iron_roots'),
(65160, 'spell_freya_iron_roots');

DELETE FROM `spell_script_names` WHERE `ScriptName`='spell_icc_stoneform';
DELETE FROM `spell_script_names` WHERE `ScriptName`='spell_icc_sprit_alarm';
INSERT INTO `spell_script_names` (`spell_id`,`ScriptName`) VALUES
(70733, 'spell_icc_stoneform'),
(70546, 'spell_icc_sprit_alarm'),
(70536, 'spell_icc_sprit_alarm'),
(70545, 'spell_icc_sprit_alarm'),
(70547, 'spell_icc_sprit_alarm');

DELETE FROM `spell_script_names` WHERE `ScriptName`='spell_baltharus_enervating_brand';
INSERT INTO `spell_script_names` (`spell_id`,`ScriptName`) VALUES
(74502, 'spell_baltharus_enervating_brand');

DELETE FROM `spell_script_names` WHERE `ScriptName`= 'spell_baltharus_enervating_brand_trigger';
INSERT INTO `spell_script_names` VALUES
(74505, 'spell_baltharus_enervating_brand_trigger');

DELETE FROM `spell_script_names` WHERE `ScriptName`= 'spell_saviana_conflagration_init';
DELETE FROM `spell_script_names` WHERE `ScriptName`= 'spell_saviana_conflagration_throwback';
INSERT INTO `spell_script_names` (`spell_id`,`ScriptName`) VALUES
(74452, 'spell_saviana_conflagration_init'),
(74455, 'spell_saviana_conflagration_throwback');
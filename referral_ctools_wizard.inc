<?php

global $user;

/**
* Create callback for standard ctools registration wizard.
*/
function dispute_referral_ctools_wizard($step = 'outline') {
  // Include required ctools files.
  ctools_include('wizard');
  ctools_include('object-cache');
  
  // Set up the multi-step wizard   
  $form_info = array(
    // Specify unique form id for this form.
    'id' => 'multistep_dispute_referral',
    //Specify the path for this form. It is important to include space for the $step argument to be passed.
    'path' => "referral/dispute/%step",
    // Show breadcrumb trail.
    'show trail' => TRUE,
    'show back' => TRUE,
    'show return' => FALSE,
    'show cancel' => TRUE,
    // Callback to use when the 'next' button is clicked.
    'next callback' => 'dispute_referral_subtask_next',
    // Callback to use when the 'previous' button is clicked.
    'back callback' => 'dispute_referral_subtask_back',
    // Callback to use when entire form is completed.
    'finish callback' => 'dispute_referral_subtask_finish',
    // Callback to use when user clicks final submit button.
    'return callback' => 'dispute_referral_subtask_finish',
    // Callback to use when user cancels wizard.
    'cancel callback' => 'dispute_referral_subtask_cancel',
    // Specify the order that the child forms will appear in, as well as their page titles.
    'order' => array(
      'outline' => t('Outline'),
      'parties' => t('Parties'),
      'features' => t('Features'),
      'agencies' => t('Agencies'),
    ),
    // Define the child forms. Be sure to use the same keys here that were user in the 'order' section of this array.
    // Be sure to load the required include file if the form callback is not defined in the .module file.
    //'include' => drupal_get_path('module', 'grasmash_registration') . '/grasmash_registration_groups_form.inc',
    'forms' => array(
      'outline' => array(
        'form id' => 'referral_outline_form'
      ),
      'parties' => array(
        'form id' => 'referral_parties_form',
        
      ),
      'features' => array(
        'form id' => 'referral_features_form',
      ),
      'agencies' => array(
        'form id' => 'referral_agencies_form',
      ),
    ),
  );

  // Make cached data available within each step's $form_state array.
  $form_state['referral_object'] = dispute_referral_get_page_cache('case_data');
 
  // Return the form as a Ctools multi-step form.
  $output = ctools_wizard_multistep_form($form_info, $step, $form_state);
 
  return $output;
}

/**
* Retreives an object from the Ctools cache.
*
* @param string $name
*  The name of the cached object to retreive.
*/
function dispute_referral_get_page_cache($name) {
  ctools_include('object-cache');
  $cache = ctools_object_cache_get('dispute_referral', $name);
 
// If the cached object doesn't exist yet, create an empty object.
  if (!$cache) {
    $cache = new stdClass();
    $cache->locked = ctools_object_cache_test('dispute_referral', $name);
  }
  return $cache;
}

/**
* Creates or updates an object in the cache.
*
* @param string $name
*  The name of the object to cache.
*
* @param object $data
*  The object to be cached.
*/
function dispute_referral_set_page_cache($name, $data) {
  ctools_include('object-cache');
  $cache = ctools_object_cache_set('dispute_referral', $name, $data);
}

/**
* Removes an item from the object cache.
*
* @param string $name
*  The name of the object to destroy.
*/
function dispute_referral_clear_page_cache($name) {
  ctools_include('object-cache');
  ctools_object_cache_clear('dispute_referral', $name);
}

/**
* Callback executed when the 'next' or 'back' button is clicked.
*/
function dispute_referral_subtask_next(&$form_state) {
  global $language;
  $lang_name = $language->language;
  
  if (!isset($form['#parents'])) {
    $form['#parents'] = array();
  }
  
  // Determine which form we are leaving so we can cache its data
  $from_form = $form_state['step'];
  
  // Set the cache status to be loaded for a new step
  $cache = dispute_referral_get_page_cache('case_data');
  $cache['uncached'] = FALSE;
  dispute_referral_set_page_cache('case_data', $cache);
  switch ($from_form){
    case 'outline':
	  $last_values = $form_state['values'];
      unset($last_values['form_build_id']);
      unset($last_values['form_token']);
      unset($last_values['form_id']);
      unset($last_values['op']);
      $last_values['outline_cached'] = TRUE;
	  // Store submitted data in a ctools cache object, namespaced 'case_data'.
      $cache = dispute_referral_get_page_cache('case_data');
      if (is_array($cache)) {
        dispute_referral_set_page_cache('case_data', array_merge($cache, $last_values));
      }
      else {
  	    dispute_referral_set_page_cache('case_data', $last_values);
      }
	  break;
	case 'parties':
	  $subform_key = key($form_state['inline_entity_form']);
      $party_cache = array(
        'field_party' => array(
          'parties' => $form_state['inline_entity_form'][$subform_key]['entities'],
	    ),
      );
      $cache = dispute_referral_get_page_cache('case_data');
      if (is_array($cache)) {
        dispute_referral_set_page_cache('case_data', array_merge($cache, $party_cache));
      }
      else {
  	    dispute_referral_set_page_cache('case_data', $party_cache);
      }
	  break;
	case 'features':
	  // Store submitted data in a ctools cache object, namespaced 'case_data'.
      $last_values = $form_state['values'];
      unset($last_values['form_build_id']);
      unset($last_values['form_token']);
      unset($last_values['form_id']);
      unset($last_values['op']);
      $last_values['features_cached'] = TRUE;
      $cache = dispute_referral_get_page_cache('case_data');
      if (is_array($cache)) {
        dispute_referral_set_page_cache('case_data', array_merge($cache, $last_values));
      }
      else {
  	    dispute_referral_set_page_cache('case_data', $last_values);
      } 
	  break;
	case 'agencies':
	  // Store submitted data in a ctools cache object, namespaced 'case_data'.
      $last_values = $form_state['values'];
      unset($last_values['form_build_id']);
      unset($last_values['form_token']);
      unset($last_values['form_id']);
      unset($last_values['op']);
      $last_values['agencies_cached'] = TRUE;
	    
	  $subform_key = key($form_state['inline_entity_form']);
      $contact_cache = array(
        'field_agency_contacts' => array(
          'contacts' => $form_state['inline_entity_form'][$subform_key]['entities'],
	    ),
      );
      $cache = dispute_referral_get_page_cache('case_data');
      if (is_array($cache)) {
        dispute_referral_set_page_cache('case_data', array_merge($cache, $last_values, $contact_cache));
      }
      else {
  	    dispute_referral_set_page_cache('case_data', array_merge($last_values, $contact_cache));
      }
	  $cache = dispute_referral_get_page_cache('case_data');
	  break;
  }
}

function dispute_referral_subtask_back(&$form_state) {
}

/**
* Callback executed when the 'cancel' button is clicked.
*/
function dispute_referral_subtask_cancel(&$form_state) {
  // Delete any attached nodes
  if (isset($form_state['inline_entity_form'])) {
    $subform_key = key($form_state['inline_entity_form']);
    $form_state['inline_entity_form'][$subform_key]['entities'] = array();
  }
  // Clear our ctools cache object. It's good housekeeping.
  dispute_referral_clear_page_cache('case_data');
  $form_state['rebuild'] = TRUE;
}

/**
* Callback executed when the entire form submission is finished.
*/
function dispute_referral_subtask_finish(&$form_state) {
  global $user;
  
  $u_wrapper = entity_metadata_wrapper('user', $user);
  
  // entity_create replaces the procedural steps in the first example of
  // creating a new object $node and setting its 'type' and uid property
  $node_values = array(
    'type' => 'case_management_case',
    'uid' => $user->uid,
    'status' => 1,
    'comment' => 0,
    'promote' => 0,
  );
  $entity = entity_create('node', $node_values);

  // Now create an entity_metadata_wrapper around the new node entity
  // to make getting and setting values easier
  $ewrapper = entity_metadata_wrapper('node', $entity);
  $cache = dispute_referral_get_page_cache('case_data');

  // Using the wrapper, we do not have to worry about telling Drupal
  // what language we are using. The Entity API handles that for us.
  $case_title = $u_wrapper->field_organisation->name->value() . ' - ';
  $case_title .= $u_wrapper->field_last_name->value() . ' - ';
  $case_title .= substr(sha1($u_wrapper->field_first_name->value() . time()), 0, 6);
  $ewrapper->title->set($case_title);

  // Setting the body is a bit different from other properties or fields
  // because the body can have both its complete value and its summary
  $ewrapper->body->set(array('value' => $cache['dispute_summary']));

  $referred_date = new DateTime();
  $entity->field_referred_date[LANGUAGE_NONE][0] = array(
    'value' => date_format($referred_date, 'Y-m-d'),
    'timezone' => drupal_get_user_timezone(),
    'timezone_db' => date_default_timezone(),
  );

  $origin_date = new DateTime($cache['dispute_age']);
  $entity->field_origin_date[LANGUAGE_NONE][0] = array(
    'value' => date_format($origin_date, 'Y-m-d'),
    'timezone' => drupal_get_user_timezone(),
    'timezone_db' => date_default_timezone(),
  );

  $case_status_term = taxonomy_get_term_by_name('referred', 'case_status');
  $ewrapper->field_case_status->set(key($case_status_term));

  $vocab = taxonomy_vocabulary_machine_name_load('dispute_factors');
  $tree = taxonomy_get_tree($vocab->vid, 0, 1, FALSE);
  array_walk($tree, '_cut_down_to_tids');
  $factor_tids = array();
  foreach($cache['dispute_factors'] as $key => $setting) {
    if (is_string($setting)) {
  	  $factor_tids[] = $tree[$key];
    }
  }
  $ewrapper->field_factor_summary->set($factor_tids);

  $vocab = taxonomy_vocabulary_machine_name_load('dispute_agencies');
  $tree = taxonomy_get_tree($vocab->vid, 0, 1, FALSE);
  array_walk($tree, '_cut_down_to_tids');
  $agency_tids = array();
  foreach($form_state['values']['agencies_involved'] as $key => $setting) {
    if (is_string($setting)) {
  	  $agency_tids[] = $tree[$key];
    }
  }
  $ewrapper->field_agencies_involved->set($agency_tids);
  foreach (current($cache['field_issues']) as $key=>$value){
  	if (is_array($value) && isset($value['value']) && is_string($value['value']) && $value['value'] != '') {
  	  $ewrapper->field_issues[] = $value['value'];
	}
  }
  $ewrapper->field_party_actions->set($cache['action_parties']);
  $ewrapper->field_agency_actions->set($cache['action_agencies']);
  $ewrapper->field_violence_summary->set($cache['dispute_violence']);
  $ewrapper->field_violence_details->set($cache['dispute_violence_details']);
  $ewrapper->field_asb_summary->set($cache['dispute_asb']);
  $ewrapper->field_asb_details->set($cache['dispute_asb_details']);
  $ewrapper->field_factor_details->set($cache['dispute_factors_details']);

  $parties_list = array();
  foreach ($cache['field_party']['parties'] as $key => $entity_def) {
  	$p_entity = entity_create('node', (array)$entity_def['entity']);
    $p_wrapper = entity_metadata_wrapper('node', $p_entity);
	$p_wrapper->save();
	$parties_list[] = array('target_id' => intval($p_wrapper->nid->value()));
	$ewrapper->field_party->set($parties_list); 
  }
  $contacts_list = array();
  foreach (current($form_state['inline_entity_form'])['entities'] as $key => $entity_def) {
  	$c_entity = entity_create('node', (array)$entity_def['entity']);
    $c_wrapper = entity_metadata_wrapper('node', $c_entity);
	$c_wrapper->save();
	$contacts_list[] = array('target_id' => intval($c_wrapper->nid->value()));
	$ewrapper->field_agency_contacts->set($contacts_list); 
  }
  $ewrapper->save();
 
  //Delete any attached nodes
  if (isset($form_state['inline_entity_form'])) {
    $subform_key = key($form_state['inline_entity_form']);
    $form_state['inline_entity_form'][$subform_key]['entities'] = array();
  }
  
  //Clear out the data held in 'referral_object' key of $form_state
  unset($form_state['referral_object']);
  
  //Clear Ctool cache object.
  dispute_referral_clear_page_cache('case_data');

  // Redirect the user to the front page.
  drupal_goto('<front>');
  }

/**
* Functions for the Outline sub-form
*/

/**
* Main form function for referral outline
*/
function referral_outline_form($form, &$form_state) {
  global $language;
  $lang_name = $language->language;
  if (!isset($form['#parents'])) {
    $form['#parents'] = array();
  }
  
  //get the saved stuff out of cache
  $cache = dispute_referral_get_page_cache('case_data');
  if (is_array($cache) && isset($cache['outline_cached']) && !$cache['uncached']) {
  	if (isset($form_state['values'])){
  	  $form_state['values'] = array_merge($form_state['values'], $cache);
    }
    else {
      $form_state['values'] = $cache;
    }
	$cached = TRUE;
  }
  else {
  	$cache['uncached'] = TRUE;
  	$cached = FALSE;
  }
  $form['dispute_summary'] = array(
    '#type' => 'textarea',
    '#title' => t('Summary of dispute'),
    '#weight' => '1',
  );
  $form['dispute_summary']['#default_value'] = ($cached) ? $form_state['values']['dispute_summary'] : '';
  
  $form['dispute_age'] = array(
   '#type' => 'date_select', // types 'date_text' and 'date_timezone' are also supported. See .inc file.
   '#title' => t('Dispute started (if known)'),
   '#default_value' => NULL, 
   '#date_format' => 'Y-m',
   '#date_timezone' => 'Europe/London', // Optional, if your date has a timezone other than the site timezone.
   '#date_year_range' => '-20:0', // Optional, used to set the year range (back 3 years and forward 3 years is the default).
   '#weight' => '2',
  );
  $form['dispute_age']['#default_value'] = ($cached) ? $form_state['values']['dispute_age'] : NULL ;
  
  if ($cached) {
  	$items = $form_state['values']['field_issues'][$lang_name];
	unset ($items['add_more']);
	$items = array_filter($items, function($e){
  	  return(is_array($e) && is_string($e['value']) && $e['value'] != '');
    });
  }
  else {
  	$items = NULL;
	$max_delta = 0;
  }
  
  $instance = field_info_instance('node', 'field_issues', 'case_management_case');
  $field = field_info_field_by_id($instance['field_id']);
  $form_element = field_default_form('node', NULL, $field, $instance, $lang_name, $items, $form, $form_state);
  $form_element['field_issues']['#weight'] = '3';
  $form = array_merge($form, $form_element);
  
  $form['action_parties'] = array(
    '#type' => 'textarea',
    '#title' => t('Action taken by parties to resolve'),
    '#weight' => '4',
  );
  $form['action_parties']['#default_value'] = ($cached) ? $form_state['values']['action_parties'] : '';
  
  $form['action_agencies'] = array(
    '#type' => 'textarea',
    '#title' => t('Action taken by agencies'),
    '#weight' => '5',
  );
  $form['action_agencies']['#default_value'] = ($cached) ? $form_state['values']['action_agencies'] : '';
  return $form;
}

/**
* Implements hook_form_FORM_ID_alter
*/
function referral_form_referral_outline_form_alter (&$form, &$form_state, $form_id) {
  _fix_subform_back_button($form);
}

/**
* Validation handler for referral outline form
*/
function referral_outline_form_validate ($form, &$form_state) {
  
}

/**
* Submit handler for referral outline form
*/
function referral_outline_form_submit ($form, &$form_state) {
  
}

/**
* Functions for the Parties sub-form
*/

/**
* Main form function for referral parties
*/
function referral_parties_form($form, &$form_state) {
  global $language;
  global $user;
  $lang_name = $language->language;
  if (!isset($form['#parents'])) {
    $form['#parents'] = array();
  }
  
  dpm($form_state);
  
  $items = NULL;
  $instance = field_info_instance('node', 'field_party', 'case_management_case');
  $field = field_info_field_by_id($instance['field_id']);
  $cache = dispute_referral_get_page_cache('case_data');
  if (!isset($form_state['inline_entity_form'])) {
  	$form_element = field_default_form('node', NULL, $field, $instance, $lang_name, $items, $form, $form_state);
  }
  $sub_key = key($form_state['inline_entity_form']);
  if (is_array($cache) && isset($cache['field_party']) && !$cache['uncached']) {
	$form_state['inline_entity_form'][$sub_key]['entities'] = $cache['field_party']['parties'];
	$cache['uncached'] = TRUE;
  }
  $form_element = field_default_form('node', NULL, $field, $instance, $lang_name, $items, $form, $form_state);
  $form_element['field_party']['#weight'] = '1';
  $form = array_merge($form, $form_element);
  dispute_referral_set_page_cache('case_data', $cache);
  return $form;
}

/**
* Implements hook_form_FORM_ID_alter 
*/
function referral_form_referral_parties_form_alter (&$form, &$form_state, $form_id) {
  _fix_subform_back_button($form);
 
  // Use #states to make visibility of 'language' box depend on 'interpreter' box being checked
  if (isset($form['field_party']['en']['form']['field_language']['und']['0']['value']['#type'])) {
    $form['field_party']['en']['form']['field_language']['und']['0']['value']['#states'] = array(
      'visible' => array(
	    "input[id='edit-field-party-en-form-field-interpreter-und']" => array('checked' => TRUE),
	  ),
    );
  }
  // We don't need the publish option on the form 
  if (isset($form['field_party']['en']['form']['status']['#type'])) {
    unset($form['field_party']['en']['form']['status']);
  }
}

/**
* Validation handler for referral parties form
*/
function referral_parties_form_validate($form, &$form_state) {
  
}

/**
* Submit handler for referral parties form
*/
function referral_parties_form_submit($form, &$form_state) {
  
}

/**
* Helper function for array_udiff in submit handler
* @params: $a1, $a2 arrays of form array('target_id' => n) to be compared
*/
function compare_keys($a1, $a2) {
  return ($a1['target_id'] - $a2['target_id']);
}

/**
* Functions for the Features sub-form
*/

/**
* Main form function for referral features
*/
function referral_features_form($form, &$form_state) {
  global $language;
  $lang_name = $language->language;
  if (!isset($form['#parents'])) {
    $form['#parents'] = array();
  }
  
  $cache = dispute_referral_get_page_cache('case_data');
  if (is_array($cache) && isset($cache['features_cached']) && !$cache['uncached']) {
  	if (isset($form_state['values'])){
  	  $form_state['values'] = array_merge($form_state['values'], $cache);
    }
    else {
      $form_state['values'] = $cache;
	}
    $cached = TRUE;
  }
  else {
  	$cache['uncached'] = TRUE;
  	$cached = FALSE;
  }
  
  $form['dispute_violence'] = array(
    '#type' => 'select',
    '#title' => t('Violence?'),
    '#weight' => '1',
    '#empty_option' => '- select -',
    '#options' => array(
      'yes' => t('Yes'), 
      'threats' => t('Threatened'), 
      'no' => t('No'),
    ),
  );
  $form['dispute_violence']['#default_value'] = ($cached) ? $form_state['values']['dispute_violence'] : '';
  
  $form['dispute_violence_details'] = array(
    '#type' => 'textarea',
    '#title' => t('Details of violence'),
    '#weight' => '2',
    '#states' => array(
      'visible' => array(
	    array("select[id='edit-dispute-violence']" => array('value' => 'yes')),
	    array("select[id='edit-dispute-violence']" => array('value' => 'threats')),
	  ),
    ),
  );
  $form['dispute_violence_details']['#default_value'] = ($cached) ? $form_state['values']['dispute_violence_details'] : '';
  
  $form['dispute_asb'] = array(
    '#type' => 'select',
    '#title' => t('Anti-social behaviour?'),
    '#weight' => '3',
    '#empty_option' => '- select -',
    '#options' => array(
      'yes' => t('Yes'), 
      'no' => t('No'),
    ),
  );
  $form['dispute_asb']['#default_value'] = ($cached) ? $form_state['values']['dispute_asb'] : '';
  
  $form['dispute_asb_details'] = array(
    '#type' => 'textarea',
    '#title' => t('Details of ASB'),
    '#weight' => '4',
    '#states' => array(
      'visible' => array(
	    "select[id='edit-dispute-asb']" => array('value' => 'yes'),
	  ),
    ),
  );
  $form['dispute_asb_details']['#default_value'] = ($cached) ? $form_state['values']['dispute_asb_details'] : '';
  
  $vocab = taxonomy_vocabulary_machine_name_load('dispute_factors');
  $tree = taxonomy_get_tree($vocab->vid, 0, 1, FALSE);
  array_walk($tree, '_cut_down_to_names');
  $form['dispute_factors'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Factors'),
    '#weight' => '5',
    '#options' => $tree,
    '#description' => 'Indicate which (if any) are factors in this dispute.',
  );
  array_walk($tree, '_unset_to_zeros');
  $form['dispute_factors']['#default_value'] = ($cached) ? $form_state['values']['dispute_factors'] : $tree;
  
  $form['dispute_factors_details'] = array(
    '#type' => 'textarea',
    '#title' => t('Further information'),
    '#description' => t('Tell us how the selected factors affect this dispute'),
    '#weight' => '6',
    '#states' => array(
      'visible' => array(
        array(':input[name="dispute_factors[0]"]' => array('checked' => TRUE)),
	    array(':input[name="dispute_factors[1]"]' => array('checked' => TRUE)),
	    array(':input[name="dispute_factors[2]"]' => array('checked' => TRUE)),
	    array(':input[name="dispute_factors[3]"]' => array('checked' => TRUE)),
	    array(':input[name="dispute_factors[4]"]' => array('checked' => TRUE)),
	  ),
    ),
  );
  
  $form['dispute_factors_details']['#default_value'] = ($cached) ? $form_state['values']['dispute_factors_details'] : '';
  
  return $form;
}

function referral_form_referral_features_form_alter (&$form, &$form_state, $form_id) {
  _fix_subform_back_button($form);
}

function referral_features_form_validate (&$form, &$form_state) {
  
}

function referral_features_form_submit (&$form, &$form_state) {
  
}

function referral_agencies_form($form, &$form_state) {
  global $language;
  $lang_name = $language->language;
  if (!isset($form['#parents'])) {
    $form['#parents'] = array();
  }
  
  $cache = dispute_referral_get_page_cache('case_data');
  if (is_array($cache) && isset($cache['agencies_cached']) && !$cache['uncached']) {
  	if (isset($form_state['values'])){
  	  $form_state['values'] = array_merge($form_state['values'], $cache);
    }
    else {
      $form_state['values'] = $cache;
	}
    $cached = TRUE;
  }
  else {
  	$cache['uncached'] = TRUE;
  	$cached = FALSE;
  }
  $vocab = taxonomy_vocabulary_machine_name_load('dispute_agencies');
  $tree = taxonomy_get_tree($vocab->vid, 0, 1, FALSE);
  array_walk($tree, '_cut_down_to_names');
  
  $form['agencies_involved'] = array(
    '#type' => 'checkboxes',
    '#title' => 'Agencies involved',
    '#weight' => '1',
    '#options' => $tree,
  );
  
  array_walk($tree, function(&$value, $key){
  	$value = 0;
  });
  
  $form['agencies_involved']['#default_value'] = ($cached) ? $form_state['values']['agencies_involved'] : $tree;
  
  $form['other_agency_name'] = array(
    '#type' => 'textfield',
    '#title' => 'Other agency name(s)',
    '#weight' => '2',
    '#states' => array(
      'visible' => array(
        ':input[name="agencies_involved[9]"]' => array('checked' => TRUE),
	  ),
    ),
  );
  
  $form['agency_contact_intro'] = array(
    '#markup' => '<p>If you want us to inform other organisations of the outcome of mediation, please provide names and contact details below.</p>',
    '#weight' => '3',
  );
  
  $form['other_agency_name']['#default_value'] = ($cached) ? $form_state['values']['other_agency_name'] : '';
  
  $items = NULL;
  $instance = field_info_instance('node', 'field_agency_contacts', 'case_management_case');
  $field = field_info_field_by_id($instance['field_id']);
  $cache = dispute_referral_get_page_cache('case_data');
  if (!isset($form_state['inline_entity_form'])) {
  	$form_element = field_default_form('node', NULL, $field, $instance, $lang_name, $items, $form, $form_state);
  }
  $sub_key = key($form_state['inline_entity_form']);
  if (is_array($cache) && isset($cache['field_agency_contacts']) && !$cache['uncached']) {
	$form_state['inline_entity_form'][$sub_key]['entities'] = $cache['field_agency_contacts']['contacts'];
	$cache['uncached'] = TRUE;
  }
  $form_element = field_default_form('node', NULL, $field, $instance, $lang_name, $items, $form, $form_state);
  $form_element['field_agency_contacts']['#weight'] = '4';
  $form = array_merge($form, $form_element);
  dispute_referral_set_page_cache('case_data', $cache);
  return $form;
}

function referral_form_referral_agencies_form_alter (&$form, &$form_state, $form_id) {
  _fix_subform_back_button($form);
}

function referral_agencies_form_validate (&$form, &$form_state) {
  
}

function referral_agencies_form_submit (&$form, &$form_state) {
  
}

function _remove_blanks($item) {
  return (!is_null($item['value']) && $item['value'] != '');
}

function _fix_subform_back_button(&$form) {
  if (isset($form['buttons']['previous'])) {
    $form['buttons']['previous']['#limit_validation_errors'] = FALSE;
	unset($form['buttons']['previous']['#submit']);
	$form['buttons']['previous']['#wizard type'] = 'next';
  }
}

function _cut_down_to_names(&$value, $key) {
  $value = t($value->name);
}

function _cut_down_to_tids(&$value, $key) {
  $value = ($value->tid);
}

function _unset_to_zeros(&$value, $key) {
  $value = 0;
}


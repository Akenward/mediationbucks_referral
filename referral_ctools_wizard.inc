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
    'return callback' => 'dispute_referral_subtask_return',
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
    $cache->locked = ctools_object_cache_test('dispute_referral', 'case_data');
  }
    
// If the cached object doesn't exist yet, create an empty object.
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
  // Determine which form we are leaving so we can cache its data
  $from_form = $form_state['step'];
  
  switch ($from_form){
    case 'outline':
	  cache_step($form_state);
	  break;
	case 'parties':
	  cache_step($form_state, 'field_party');
	  break;
	case 'features':
	  cache_step($form_state);
	  $cache = dispute_referral_get_page_cache('case_data');
	  if (isset($cache->confirm_routed)) {
	  	unset($cache->confirm_routed);
	  }
      dispute_referral_set_page_cache('case_data', $cache);
	  break;
	case 'agencies':
	  cache_step($form_state, 'field_agency_contacts');
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
  unset($form_state['referral_object']);
  
  $form_state['rebuild'] = TRUE;
}

function dispute_referral_subtask_return(&$form_state) {

}

/**
* Callback executed when the entire form submission is finished.
*/
function dispute_referral_subtask_finish(&$form_state) {
  $node_values = array(
    'status' => 0,
    'comment' => 0,
    'promote' => 0,
  );
  
  $ewrapper = entity_metadata_wrapper('node', build_case_node($form_state, $node_values));
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
  $cache = dispute_referral_get_page_cache('case_data');
  if (!empty($_SERVER['HTTP_X_REQUESTED_WITH']) && strtolower($_SERVER['HTTP_X_REQUESTED_WITH']) == 'xmlhttprequest') {
    $cache->ajax_call = TRUE;
  }
  else {
  	$cache->ajax_call = FALSE;
  }
  dispute_referral_set_page_cache('case_data', $cache); 	
  
  // Set up control for dispute summary and get saved value from [referral_object]
  // if it exists and if this is the first display of the form for this step
  $form['dispute_summary'] = array(
    '#type' => 'textarea',
    '#title' => t('Summary of dispute'),
    '#required' => TRUE,
    '#weight' => '1',
    '#default_value' => _fetch_default_value('dispute_summary', $cache, $form_state, ''),
  );
  
  // Set up control for dispute age and get saved value from [referral_object]
  // if it exists and if this is the first display of the form for this step
  $form['dispute_age'] = array(
    '#type' => 'date_select', // types 'date_text' and 'date_timezone' are also supported. See .inc file.
    '#title' => t('Dispute started (if known)'),
    '#default_value' => NULL, 
    '#date_format' => 'Y-m',
    '#date_timezone' => 'Europe/London', // Optional, if your date has a timezone other than the site timezone.
    '#date_year_range' => '-20:0', // Optional, used to set the year range (back 3 years and forward 3 years is the default).
    '#weight' => '2',
    '#default_value' => _fetch_default_value('dispute_age', $cache, $form_state),
  );
  
  // Set up control for dispute issues and get saved value from [referral_object]
  // if it exists and if this is the first display of the form for this step
  $items = _fetch_default_value('field_issues', $cache, $form_state);
  if (is_array($items)) {
	$items = current($items);
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
  $form_element['field_issues'][$lang_name]['add_more']['#value'] = 'Add another issue';
  $form = array_merge($form, $form_element);
  
  // Set up control for action_parties and get saved value from [referral_object]
  // if it exists and if this is the first display of the form for this step
  $form['action_parties'] = array(
    '#type' => 'textarea',
    '#title' => t('Action taken by parties to resolve'),
    '#weight' => '4',
    '#default_value' => _fetch_default_value('action_parties', $cache, $form_state, ''),
  );
  
  // Set up control for action_agencies and get saved value from [referral_object]
  // if it exists and if this is the first display of the form for this step
  $form['action_agencies'] = array(
    '#type' => 'textarea',
    '#title' => t('Action taken by agencies'),
    '#weight' => '5',
    '#default_value' => _fetch_default_value('action_agencies', $cache, $form_state, ''),
  );
  $form['#attached']['js'] = array(
    drupal_get_path('module', 'referral') . '/referral.js',
  );
  
  dispute_referral_set_page_cache('case_data', $cache);
  return $form;
}

/**
* Implements hook_form_FORM_ID_alter
*/
function referral_form_referral_outline_form_alter (&$form, &$form_state, $form_id) {
  global $language;
  $lang_name = $language->language;
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
  $cache = dispute_referral_get_page_cache('case_data');
  if (!empty($_SERVER['HTTP_X_REQUESTED_WITH']) && strtolower($_SERVER['HTTP_X_REQUESTED_WITH']) == 'xmlhttprequest') {
    $cache->ajax_call = TRUE;
  }
  else {
  	$cache->ajax_call = FALSE;
  }
  dispute_referral_set_page_cache('case_data', $cache);
  
  $items = NULL;
  $instance = field_info_instance('node', 'field_party', 'case_management_case');
  $field = field_info_field_by_id($instance['field_id']);
  if (!isset($form_state['inline_entity_form'])) {
  	$form_element = field_default_form('node', NULL, $field, $instance, $lang_name, $items, $form, $form_state);
  }
  $sub_key = key($form_state['inline_entity_form']);
  if ($parties = _fetch_default_value('field_party', $cache, $form_state)) {
	if (in_array('entity', current($parties))){
      $form_state['inline_entity_form'][$sub_key]['entities'] = $parties;
	}
  }
  $form_element = field_default_form('node', NULL, $field, $instance, $lang_name, $items, $form, $form_state);
  $form_element['field_party']['#weight'] = '1';
  // We don't need the publish option on the form 
  if (isset($form_element['field_party'][$lang_name]['form'])) {
    unset($form_element['field_party'][$lang_name]['form']['status']);
	$form_element['field_party'][$lang_name]['form']['#title'] = 'Add new party';
	$form_element['field_party'][$lang_name]['form']['actions']['ief_add_save']['#value'] = 'Create party';
	$form_element['field_party'][$lang_name]['form']['field_residents']['und']['#empty_option'] = '- Select a value -';
  }
  $form_element['field_party'][$lang_name]['actions']['ief_add']['#value'] = 'Add a party';
  $form = array_merge($form, $form_element);
  $form['#attached']['js'] = array(
    drupal_get_path('module', 'referral') . '/referral.js',
  );
  
  dispute_referral_set_page_cache('case_data', $cache);
  return $form;
}

/**
* Implements hook_form_FORM_ID_alter 
*/
function referral_form_referral_parties_form_alter (&$form, &$form_state, $form_id) {
  global $language;
  $lang_name = $language->language;
  _fix_subform_back_button($form);
 
  // Use #states to make visibility of 'language' box depend on 'interpreter' box being checked
  if (isset($form['field_party']['en']['form']['field_language']['und']['0']['value']['#type'])) {
    $form['field_party']['en']['form']['field_language']['und']['0']['value']['#states'] = array(
      'visible' => array(
	    "input[id='edit-field-party-en-form-field-interpreter-und']" => array('checked' => TRUE),
	  ),
    );
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
  if (!empty($_SERVER['HTTP_X_REQUESTED_WITH']) && strtolower($_SERVER['HTTP_X_REQUESTED_WITH']) == 'xmlhttprequest') {
    $cache->ajax_call = TRUE;
  }
  else {
  	$cache->ajax_call = FALSE;
  }
  dispute_referral_set_page_cache('case_data', $cache);
  
  $form['dispute_violence'] = array(
    '#type' => 'select',
    '#title' => t('Violence?'),
    '#required' => TRUE,
    '#weight' => '1',
    '#empty_option' => '- select -',
    '#options' => array(
      'yes' => t('Yes'), 
      'threats' => t('Threatened'), 
      'no' => t('No'),
    ),
    '#default_value' => _fetch_default_value('dispute_violence', $cache, $form_state),
  );
  
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
    '#default_value' => _fetch_default_value('dispute_violence_details', $cache, $form_state, ''),
  );
  
  $form['dispute_asb'] = array(
    '#type' => 'select',
    '#title' => t('Anti-social behaviour?'),
    '#required' => TRUE,
    '#weight' => '3',
    '#empty_option' => '- select -',
    '#options' => array(
      'yes' => t('Yes'), 
      'no' => t('No'),
    ),
    '#default_value' => _fetch_default_value('dispute_asb', $cache, $form_state),
  );
  
  $form['dispute_asb_details'] = array(
    '#type' => 'textarea',
    '#title' => t('Details of ASB'),
    '#weight' => '4',
    '#states' => array(
      'visible' => array(
	    "select[id='edit-dispute-asb']" => array('value' => 'yes'),
	  ), 
    ),
    '#default_value' => _fetch_default_value('dispute_asb_details', $cache, $form_state, ''),
  );
  
  $vocab = taxonomy_vocabulary_machine_name_load('dispute_factors');
  $tree = taxonomy_get_tree($vocab->vid, 0, 1, FALSE);
  array_walk($tree, '_cut_down_to_names');
  $tree = array_combine(range(1, count($tree)), array_values($tree));
  $null_tree = $tree;
  array_walk($null_tree, '_unset_to_zeros');
  $form['dispute_factors'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Factors'),
    '#weight' => '5',
    '#options' => $tree,
    '#description' => 'Indicate which (if any) are factors in this dispute.',
    '#default_value' => _fetch_default_value('dispute_factors', $cache, $form_state, $null_tree),
  );
  
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
    '#default_value' => _fetch_default_value('dispute_factors_details', $cache, $form_state, ''),
  );
  
  dispute_referral_set_page_cache('case_data', $cache);
  
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
  dispute_referral_set_page_cache('case_data', $cache);
  $form['#attached']['js'] = array(
    drupal_get_path('module', 'referral') . '/referral.js',
  );
  $vocab = taxonomy_vocabulary_machine_name_load('dispute_agencies');
  $tree = taxonomy_get_tree($vocab->vid, 0, 1, FALSE);
  array_walk($tree, '_cut_down_to_names');
  $tree = array_combine(range(1, count($tree)), array_values($tree));
  $other_key = array_search ( 'Other' , $tree );
  $null_tree = $tree;
  array_walk($null_tree, '_unset_to_zeros');
  $form['agencies_involved'] = array(
    '#type' => 'checkboxes',
    '#title' => 'Agencies involved',
    '#weight' => '1',
    '#options' => $tree,
    '#default_value' => _fetch_default_value('agencies_involved', $cache, $form_state, $null_tree),
  );
  
  $form['other_agency_name'] = array(
    '#type' => 'textfield',
    '#title' => 'Other agency name(s)',
    '#weight' => '2',
    '#states' => array(
      'visible' => array(
        ':input[name="agencies_involved[' . $other_key .']"]' => array('checked' => TRUE),
	  ),
    ),
    '#default_value' => _fetch_default_value('other_agency_name', $cache, $form_state, ''),
  );
  
  $form['agency_contact_intro'] = array(
    '#markup' => '<p>If you want us to inform other organisations of the outcome of mediation, please provide names and contact details below.</p>',
    '#weight' => '3',
  );
  
  $items = NULL;
  $instance = field_info_instance('node', 'field_agency_contacts', 'case_management_case');
  $field = field_info_field_by_id($instance['field_id']);
  if (!isset($form_state['inline_entity_form'])) {
  	$form_element = field_default_form('node', NULL, $field, $instance, $lang_name, $items, $form, $form_state);
  }
  $sub_key = key($form_state['inline_entity_form']);
  if ($contacts = _fetch_default_value('field_agency_contacts', $cache, $form_state)) {
	if (in_array('entity', current($contacts))){
      $form_state['inline_entity_form'][$sub_key]['entities'] = $contacts;
	}
  }
  $form_element = field_default_form('node', NULL, $field, $instance, $lang_name, $items, $form, $form_state);
  $form_element['field_agency_contacts']['#weight'] = '4';
  if (isset($form_element['field_agency_contacts'][$lang_name]['form'])) {
    unset($form_element['field_agency_contacts'][$lang_name]['form']['status']);
	$form_element['field_agency_contacts'][$lang_name]['form']['#title'] = 'Add a contact';
	$form_element['field_agency_contacts'][$lang_name]['form']['actions']['ief_add_save']['#value'] = 'Create contact';
  }
  if (isset($form_element['field_agency_contacts'][$lang_name]['actions']['ief_add'])) {
    $form_element['field_agency_contacts'][$lang_name]['actions']['ief_add']['#value'] = 'Add a contact';
  }
  $form = array_merge($form, $form_element);
  dispute_referral_set_page_cache('case_data', $cache);
  
  $path = 'referral/dispute/outline';
  
  if( !isset( $form_state['triggering_element']['#ajax'] ) && isset($cache->confirm_routed)) {
    // The form shown during the confirmation step:
    $node_values = array(
      'status' => 0,
      'comment' => 0,
      'promote' => 0,
    );
	$node = build_case_node($form_state, $node_values);
	$render_array = build_case_render_array($node);
    $form['case_info'] = array(
      '#markup' => drupal_render($render_array),
    );
    $path = 'referral/dispute/outline';
	$description = '<p>You can confirm it below or cancel to go back and edit/add to it.</p>';
	$description .= '<p>To preserve confidentiality for the parties, you will not be able to';
	$description .= 'access this referral once you have confirmed it.';
	$description .= 'However, you can opt to see a print-friendly version and/or download a pdf file for your records.</p>';
	return confirm_form($form, 
	  $question=t('Is this information correct?'), 
	  $path,
	  $description, 
	  $yes = 'Confirm'
	);
  }
  else {
  	if( !isset( $form_state['triggering_element']['#ajax'] )) {
  	  $cache->confirm_routed = TRUE;
	  dispute_referral_set_page_cache('case_data', $cache);
	}
  	return $form;
  }
}

function referral_form_referral_agencies_form_alter (&$form, &$form_state, $form_id) {
  _fix_subform_back_button($form);
  $cache = dispute_referral_get_page_cache('case_data');
  if (!isset($form_state['triggering_element']['#ajax'])){
    if (isset($cache->confirm_routed) && $cache->confirm_routed) {
  	  $form_state['plan_rebuild'] = TRUE;
	  if (isset($form['buttons']['return']['#wizard type'])) {
	    $form['buttons']['return']['#wizard type'] = 'return';
	  }
    }
    else {
  	  $form_state['plan_rebuild'] = FALSE;
	  if (isset($form['actions']['submit'])) {
	    $form['actions']['submit']['#wizard type'] = 'finish';
		unset($form['ctools_trail']);
	    unset($form['buttons']);
	    unset($form['agencies_involved']);
	    unset($form['other_agency_name']);
	    unset($form['agency_contact_intro']);
	    unset($form['field_agency_contacts']);
	  }
	  if (isset($form['buttons']['return']['#wizard type'])) {
	    $form['buttons']['return']['#wizard type'] = 'return';
	  }
  	}
  }
  $cache->confirm_routed = FALSE;
  dispute_referral_set_page_cache('case_data', $cache);
}

function referral_agencies_form_validate (&$form, &$form_state) {

}

function referral_agencies_form_submit (&$form, &$form_state) {
  $form_state['rebuild'] = $form_state['plan_rebuild'];
}

/**
* Implements drupal_alter inline_entity_form_table_fields
* @fields: arrays of entity table column settings
* @context: information to use in deciding the appropriate column e.g. field_name
*  
*/
function referral_inline_entity_form_table_fields_alter(&$fields, $context) {
  switch ($context['field_name']) {
    case 'field_party':
      $fields = array(
        'field_party_reference' => array(
	      'type' => 'field',
          'label' => '',
	      'weight' => 5,
        ),
        'title' => array(
          'type' => 'property',
          'label' => 'Name',
          'weight' => 10,
        ),
      );
	  break;
    case 'field_agency_contacts':
      $fields = array(
        'field_name' => array(
	      'type' => 'field',
          'label' => 'Name',
	      'weight' => 5,
        ),
        'title' => array(
          'type' => 'property',
          'label' => 'Organisation',
          'weight' => 10,
        ),
      );
	break;
  }
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

function _fetch_default_value ($field_name, $cache, $form_state, $null_value = null) {
  $cache = dispute_referral_get_page_cache('case_data');
  if (!isset($form_state['triggering_element']['#ajax']) && property_exists($cache, $field_name)) {
	return $cache->$field_name;
  }
  else {
  	return $null_value;
  }
}

function cache_step($form_state, $entity_key = null){
  if (isset($form_state['values'])) { 
    $field_values = $form_state['values'];
    unset($field_values['form_build_id']);
    unset($field_values['form_token']);
    unset($field_values['form_id']);
    unset($field_values['op']);
    unset($field_values['next']);
    unset($field_values['previous']);
    unset($field_values['cancel']);
    unset($field_values['finish']);
    // Store submitted data in a ctools cache object, namespaced 'case_data'.
    $cache = dispute_referral_get_page_cache('case_data');
    foreach ($field_values as $key => $value) {
      $cache->$key = $value;
    }
    if ($entity_key) {
      $subform_key = key($form_state['inline_entity_form']);
	  if (isset($form_state['inline_entity_form'][$subform_key]['entities'])) {
        $cache->$entity_key = $form_state['inline_entity_form'][$subform_key]['entities'];
	  }
    }
    dispute_referral_set_page_cache('case_data', $cache);
  }
}

function build_case_node(&$form_state, $options) {
  global $user;
  
  // Agencies, the last step, is most likely not in cache as this is done
  // after a move to another step.  So we do the save here.
  cache_step($form_state, 'field_agency_contacts');
  
  $u_wrapper = entity_metadata_wrapper('user', $user);
  
  // entity_create replaces the procedural steps in the first example of
  // creating a new object $node and setting its 'type' and uid property
  $node_values = array(
    'type' => 'case_management_case',
    'uid' => $user->uid,
  );
  $node_values = array_merge($node_values, $options);
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
  $body = array();
  $body['format'] = NULL;
  $body['value'] = (isset($cache->dispute_summary)) ? $cache->dispute_summary : '';
  $ewrapper->body->set($body);

  $referred_date = new DateTime();
  $entity->field_referred_date[LANGUAGE_NONE][0] = array(
    'value' => date_format($referred_date, 'Y-m-d'),
    'timezone' => drupal_get_user_timezone(),
    'timezone_db' => date_default_timezone(),
  );
  
  if (isset($cache->dispute_age)) {
    $origin_date = new DateTime($cache->dispute_age);
    $entity->field_origin_date[LANGUAGE_NONE][0] = array(
      'value' => date_format($origin_date, 'Y-m-d'),
      'timezone' => drupal_get_user_timezone(),
      'timezone_db' => date_default_timezone(),
    );
  }

  $case_status_term = taxonomy_get_term_by_name('referred', 'case_status');
  $ewrapper->field_case_status->set(key($case_status_term));
  
  if (isset($cache->dispute_factors)) {
    $vocab = taxonomy_vocabulary_machine_name_load('dispute_factors');
    $tree = taxonomy_get_tree($vocab->vid, 0, 1, FALSE);
    array_walk($tree, '_cut_down_to_tids');
    $factor_tids = array();
    foreach($cache->dispute_factors as $key => $setting) {
      if (is_string($setting)) {
  	    $factor_tids[] = $tree[$key - 1];
      }
    }
	$ewrapper->field_factor_summary->set($factor_tids);
  }
 
  if (isset($cache->agencies_involved)) {
    $vocab = taxonomy_vocabulary_machine_name_load('dispute_agencies');
    $tree = taxonomy_get_tree($vocab->vid, 0, 1, FALSE);
    array_walk($tree, '_cut_down_to_tids');
    $agency_tids = array();
    foreach($cache->agencies_involved as $key => $setting) {
      if (is_string($setting)) {
  	    $agency_tids[] = $tree[$key - 1];
      }
    }
	$ewrapper->field_agencies_involved->set($agency_tids);
  }
  
  If (isset($cache->field_issues)) {
    foreach (current($cache->field_issues) as $key=>$value){
  	  if (is_array($value) && isset($value['value']) && is_string($value['value']) && $value['value'] != '') {
  	    $ewrapper->field_issues[] = $value['value'];
	  }
    }
  }
  
  if (isset($cache->action_parties)){
  	$ewrapper->field_party_actions->set($cache->action_parties);
  }
  if (isset($cache->action_agencies)){
    $ewrapper->field_agency_actions->set($cache->action_agencies);
  }
  $ewrapper->field_violence_summary->set($cache->dispute_violence);
  if (isset($cache->dispute_violence_details)){
    $ewrapper->field_violence_details->set($cache->dispute_violence_details);
  }
  $ewrapper->field_asb_summary->set($cache->dispute_asb);
  if (isset($cache->dispute_asb_details)){
    $ewrapper->field_asb_details->set($cache->dispute_asb_details);
  }
  if (isset($cache->dispute_factors_details)){
    $ewrapper->field_factor_details->set($cache->dispute_factors_details);
  }
  
  $parties_list = array();
  $node_values['type'] = 'dispute_party';
  if (isset($cache->field_party)) {
    foreach ($cache->field_party as $key => $entity_def) {
  	  $p_entity = entity_create('node', array_merge((array)$entity_def['entity'], $node_values));
      $p_wrapper = entity_metadata_wrapper('node', $p_entity);
	  $p_wrapper->save();
	  $parties_list[] = array('target_id' => intval($p_wrapper->nid->value()));
	  $ewrapper->field_party->set($parties_list);
	}
  }
  
  $contacts_list = array();
  $node_values['type'] = 'agency_contact';
  if (isset($cache->field_agency_contacts)) {
    foreach ($cache->field_agency_contacts as $key => $entity_def) {
  	  $c_entity = entity_create('node', array_merge((array)$entity_def['entity'], $node_values));
      $c_wrapper = entity_metadata_wrapper('node', $c_entity);
	  $c_wrapper->save();
	  $contacts_list[] = array('target_id' => intval($c_wrapper->nid->value()));
	  $ewrapper->field_agency_contacts->set($contacts_list);
	}
  }
  return $entity;
}

function build_case_render_array($entity) {
  
  global $language;
  $lang_name = $language->language;
  return node_view($entity, 'full', $lang_name);
  /*$render_array = array();
  $render_array['body'] = field_view_field('node', $entity, 'body', 'full', $lang_name);
  if (isset($entity->field_origin_date)) {
  	$render_array['field_origin_date'] = field_view_field('node', $entity, 'field_origin_date', 'full', $lang_name);
  }
  if (isset($entity->field_factor_summary)) {
  	$render_array['field_factor_summary'] = field_view_field('node', $entity, 'field_factor_summary', 'full', $lang_name);
  }
  if (isset($entity->field_agencies_involved)) {
  	//$render_array['field_agencies_involved'] = field_view_field('node', $entity, 'field_agencies_involved', 'full', $lang_name);
    dpm(field_view_field('node', $entity, 'field_agencies_involved', 'full', $lang_name));
  }
  
  dpm($entity);
  
  if (isset($entity->field_issues)) {
  	$render_array['field_issues'] = field_view_field('node', $entity, 'field_issues', 'full', $lang_name);
  }
  if (isset($entity->field_party_actions)) {
  	$render_array['field_party_actions'] = field_view_field('node', $entity, 'field_party_actions', 'full', $lang_name);
  }
  if (isset($entity->field_agency_actions)) {
  	$render_array['field_agency_actions'] = field_view_field('node', $entity, 'field_agency_actions', 'full', $lang_name);
  }
  if (isset($entity->field_agency_contacts)) {
  	$render_array['field_agency_contacts'] = field_view_field('node', $entity, 'field_agency_contacts', 'full', $lang_name);
  }
  $render_array['field_violence_summary'] = field_view_field('node', $entity, 'field_violence_summary', 'full', $lang_name);
  $render_array['field_asb_summary'] = field_view_field('node', $entity, 'field_asb_summary', 'full', $lang_name);

  return $render_array;*/
}




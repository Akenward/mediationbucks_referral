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

  // Make cached data available within each step's $form_state array. Not used but
  // commented in case required in future
  //$form_state['referral_object'] = dispute_referral_get_page_cache('case_data');
 
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

}

function dispute_referral_subtask_back(&$form_state) {
	
}

/**
* Callback executed when the 'cancel' button is clicked.
*/
function dispute_referral_subtask_cancel(&$form_state) {
  // Delete the node under development.  Child nodes for parties
  // and agency contacts will be deleted automatically.
  $cache = dispute_referral_get_page_cache('case_data');
  $node = node_delete($cache->ewrapper->nid->value());

  // Clear ctools cache object.
  dispute_referral_clear_page_cache('case_data');
  
  // Redirect the user to the cancellation page.
  drupal_goto('content/referral-cancelled');
}

function dispute_referral_subtask_return(&$form_state) {
}

/**
* Callback executed when the entire form submission is finished.
*/
function dispute_referral_subtask_finish(&$form_state) {
  
  // Do updates from agencies form
  // ===> save agencies data and agency contact nodes
  // if we don't go through the agency commit handler

  $cache = dispute_referral_get_page_cache('case_data');
  $cache->ewrapper->status->set(0);
  $cache->ewrapper->save();
  //Clear Ctool cache object.
  dispute_referral_clear_page_cache('case_data');

  // Redirect the user to the confirmation page.
  drupal_goto('content/referral-complete');
  }

/**
* Functions for the Outline sub-form
*/

/**
* Implements hook_form for "outline"
*/
function referral_outline_form($form, &$form_state) {
  // create the initial case referral node that we will build on
  referral_create_referral();

  if (!isset($form['#parents'])) {
    $form['#parents'] = array();
  }
  $cache = dispute_referral_get_page_cache('case_data');
  // unset flag to prevent extra submit for agencies form (on re-entry from cancel confirm)
  unset($cache->agencies_submitted);
  
  $node = node_load($cache->ewrapper->nid->value());
  
  // Set up control for dispute summary with any existing data
  $form['dispute_summary'] = array(
    '#type' => 'textarea',
    '#title' => t('Summary of dispute'),
    '#required' => TRUE,
    '#weight' => '1',
    '#default_value' => _fetch_default_value('body', $cache, ''),
  );
  
  // Set up control for origin date with any existing data
  $items = _fetch_default_value('field_origin_date', $cache);
  $instance = field_info_instance('node', 'field_origin_date', 'case_management_case');
  $field = field_info_field_by_id($instance['field_id']);
  $form_element = field_default_form('node', $node, $field, $instance, 'und', $items, $form, $form_state);
  $form_element['field_origin_date']['#weight'] = '2';
  $form = array_merge($form, $form_element);
  
  // Set up control for dispute issues using "case issues" vocabulary as a checkbox list with any existing data 
  $items = _fetch_default_value('field_issues', $cache);
  $instance = field_info_instance('node', 'field_issues', 'case_management_case');
  $field = field_info_field_by_id($instance['field_id']);
  $form_element = field_default_form('node', $node, $field, $instance, 'und', $items, $form, $form_state);
  $form_element['field_issues']['#weight'] = '3';
  $form = array_merge($form, $form_element);
  $other_key = array_search('Other', $form_element['field_issues']['und']['#options']);
 
  // Set up control for "other issue" as a textfield visible only when "Other" is selected in
  // "case issues" with any existing data  
  $form['dispute_other_issue'] = array(
    '#type' => 'textfield',
    '#title' => t('Other issue'),
    '#weight' => '4',
    '#states' => array(
      'visible' => array(
	    'input[name="field_issues[und][' . $other_key .']"' => array('checked' => TRUE),
	  ),
    ),
    '#default_value' => _fetch_default_value('field_other_issue', $cache, ''),
  );
  
  // Set up control for action_parties with any existing data 
  $form['action_parties'] = array(
    '#type' => 'textarea',
    '#title' => t('Action taken by parties to resolve'),
    '#weight' => '5',
    '#default_value' => _fetch_default_value('field_party_actions', $cache, ''),
  );
  
  // Set up control for action_agencies with any existing data
  $form['action_agencies'] = array(
    '#type' => 'textarea',
    '#title' => t('Action taken by agencies'),
    '#weight' => '6',
    '#default_value' => _fetch_default_value('field_agency_actions', $cache, ''),
  );
  return $form;
}

/**
* Implements hook_form_FORM_ID_alter for "outline" form
*/
function referral_form_referral_outline_form_alter (&$form, &$form_state, $form_id) {
  _fix_subform_back_button($form);
}

/**
* Implements hook_form_validate for "outline" form
*/
function referral_outline_form_validate ($form, &$form_state) {
  
}

/**
* Implements hook_form_submit for "outline" form
*/
function referral_outline_form_submit ($form, &$form_state) {
  $cache = dispute_referral_get_page_cache('case_data');
  $node = node_load($cache->ewrapper->nid->value());
  
  // Update value for dispute summary
  $body = array();
  $body['format'] = NULL;
  $body['value'] = $form_state['values']['dispute_summary'];
  $cache->ewrapper->body->set($body);
  
  // Update value for origin date 
  $submit_options = array(
    'field_id' => 'field_origin_date',
    'deleted' => FALSE,
    'language' => 'und',
  );
  field_attach_submit('node', $node, $form, $form_state, $submit_options);
  
  // Update value for issues 
  $submit_options = array(
    'field_id' => 'field_issues',
    'deleted' => FALSE,
    'language' => 'und',
  );
  field_attach_submit('node', $node, $form, $form_state, $submit_options);

  // Update value for other issue
  $cache->ewrapper->field_other_issue->set($form_state['values']['dispute_other_issue']);
  
  // Update value for action_parties
  $cache->ewrapper->field_party_actions->set($form_state['values']['action_parties']);
  
  // Update value for action_agencies
  $cache->ewrapper->field_agency_actions->set($form_state['values']['action_agencies']);
  
  // Save the case referral node with new values
  $cache->ewrapper->save();
  
  }
/**
* Functions for the Parties sub-form
*/

/**
* Implements hook_form for "parties"
*/
function referral_parties_form($form, &$form_state) {
  referral_create_referral();
  global $user;
  if (!isset($form['#parents'])) {
    $form['#parents'] = array();
  }
  $cache = dispute_referral_get_page_cache('case_data');
  $node = node_load($cache->ewrapper->nid->value());
  $items = _fetch_default_value('field_party', $cache, array());
  $instance = field_info_instance('node', 'field_party', 'case_management_case');
  $field = field_info_field_by_id($instance['field_id']);
  $form_element = field_default_form('node', $node, $field, $instance, 'und', $items, $form, $form_state);
  $form_element['field_party']['#weight'] = '1';
  // We don't need the publish option on the form 
  if (isset($form_element['field_party']['und']['form'])) {
    unset($form_element['field_party']['und']['form']['status']);
	$form_element['field_party']['und']['form']['#title'] = 'Add new party';
	$form_element['field_party']['und']['form']['actions']['ief_add_save']['#value'] = 'Create party';
	$form_element['field_party']['und']['form']['field_residents']['und']['#empty_option'] = '- Select a value -';
  }
  if (isset($form['field_party']['und']['entities']['0']['form']['status'])) {
  	unset($form['field_party']['und']['entities']['0']['form']['status']);
  }
  $form_element['field_party']['und']['actions']['ief_add']['#value'] = 'Add a party';
  $form = array_merge($form, $form_element);
  $form['#attached']['js'] = array(
    drupal_get_path('module', 'referral') . '/referral.js',
  );
 
  return $form;
}

/**
* Implements hook_form_FORM_ID_alter for "parties" form
*/
function referral_form_referral_parties_form_alter (&$form, &$form_state, $form_id) {
  _fix_subform_back_button($form);
 
  // Use #states to make visibility of 'language' box depend on 'interpreter' box being checked
  if (isset($form['field_party']['und']['form']['field_language']['und']['0']['value']['#type'])) {
    $form['field_party']['und']['form']['field_language']['und']['0']['value']['#states'] = array(
      'visible' => array(
	    "input[id='edit-field-party-und-form-field-interpreter-und']" => array('checked' => TRUE),
	  ),
    );
  }
  // Do the same in the edit party form
  if (isset($form['field_party']['und']['entities']['0']['form']['field_language']['und']['0']['value']['#type'])) {
  	$form['field_party']['und']['entities']['0']['form']['field_language']['und']['0']['value']['#states'] = array(
      'visible' => array(
	    "input[id='edit-field-party-und-entities-0-form-field-interpreter-und']" => array('checked' => TRUE),
	  ),
    );
  }
  // Switch off the control for published/unpublished status
  if (isset($form['field_party']['und']['entities']['0']['form']['status'])) {
  	unset($form['field_party']['und']['entities']['0']['form']['status']);
  }
}

/**
* Implements hook_form_validate for "parties" form
*/
function referral_parties_form_validate($form, &$form_state) {
  
}

/**
* Implements hook_form_submit for "parties" form
*/
function referral_parties_form_submit($form, &$form_state) {
  $submit_options = array(
    'field_id' => 'field_party',
    'deleted' => FALSE,
  );
  $cache = dispute_referral_get_page_cache('case_data');
  $node = node_load($cache->ewrapper->nid->value());
  field_attach_submit('node', $node, $form, $form_state, $submit_options);
  $cache->ewrapper->save();
}

/**
* Functions for the Features sub-form
*/

/**
* Implements hook_form for "features"
*/
function referral_features_form($form, &$form_state) {
  referral_create_referral();
  if (!isset($form['#parents'])) {
    $form['#parents'] = array();
  }
  $cache = dispute_referral_get_page_cache('case_data');
  // unset flag to prevent extra submit for agencies form (on re-entry from agency via previous button)
  unset($cache->agencies_submitted);
  $node = node_load($cache->ewrapper->nid->value());
  
  // Set up control for dispute violence with three options  
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
    '#default_value' => _fetch_default_value('field_violence_summary', $cache),
  );
  
  // Set up control for violence details, visible if violence is "Yes" or "Threatened" 
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
    '#default_value' => _fetch_default_value('field_violence_details', $cache, ''),
  );
  
  // Set up control for dispute ASB with two options  
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
    '#default_value' => _fetch_default_value('field_asb_summary', $cache),
  );
  
  // Set up control for ASB details, visible if ASB is "Yes" 
  $form['dispute_asb_details'] = array(
    '#type' => 'textarea',
    '#title' => t('Details of ASB'),
    '#weight' => '4',
    '#states' => array(
      'visible' => array(
	    "select[id='edit-dispute-asb']" => array('value' => 'yes'),
	  ), 
    ),
    '#default_value' => _fetch_default_value('field_asb_details', $cache, ''),
  );
  
  // Set up control for dispute factors using "dispute factors" vocabulary as a checkbox list  
  $items = _fetch_default_value('field_factor_summary', $cache);
  $instance = field_info_instance('node', 'field_factor_summary', 'case_management_case');
  $field = field_info_field_by_id($instance['field_id']);
  $form_element = field_default_form('node', $node, $field, $instance, 'und', $items, $form, $form_state);
  $form_element['field_factor_summary']['#weight'] = '5';
  $form = array_merge($form, $form_element);
  
  // Set up a control for "factor details" if any of the the factors are selected
  $state_array = array();
  $vocab = taxonomy_vocabulary_machine_name_load('dispute_factors');
  $tree = taxonomy_get_tree($vocab->vid, 0, 1, FALSE);
  foreach($tree as $id => $term){
  	$state_array[] = array(':input[name="field_factor_summary[und][' . $term->tid . ']"]' => array('checked' => TRUE));
  }  
  $form['dispute_factor_details'] = array(
    '#type' => 'textarea',
    '#title' => t('Further information'),
    '#description' => t('Tell us how the selected factors affect this dispute'),
    '#weight' => '6',
    '#states' => array(
      'visible' => $state_array,
    ),
    '#default_value' => _fetch_default_value('field_factor_details', $cache, ''),
  );
  return $form;
}

/**
* Implements hook_form_FORM_ID_alter for "features" form
*/
function referral_form_referral_features_form_alter (&$form, &$form_state, $form_id) {
  _fix_subform_back_button($form);
}

/**
* Implements hook_form_validate for "features" form
*/
function referral_features_form_validate (&$form, &$form_state) {
  
}

/**
* Implements hook_form_submit for "features" form
*/
function referral_features_form_submit (&$form, &$form_state) {
  $cache = dispute_referral_get_page_cache('case_data');
  $node = node_load($cache->ewrapper->nid->value());
  
  // Update violence summary
  $cache->ewrapper->field_violence_summary->set($form_state['values']['dispute_violence']);
  
  // Update violence details
  $cache->ewrapper->field_violence_details->set($form_state['values']['dispute_violence_details']);
 
  // Update asb summary
  $cache->ewrapper->field_asb_summary->set($form_state['values']['dispute_asb']);
  
  // Update asb details
  $cache->ewrapper->field_asb_details->set($form_state['values']['dispute_asb_details']);
  
  // Update value for factor summary
  $submit_options = array(
    'field_id' => 'field_factor_summary',
    'deleted' => FALSE,
    'language' => 'und',
  );
  field_attach_submit('node', $node, $form, $form_state, $submit_options);
  
  // Update factor details
  $cache->ewrapper->field_factor_details->set($form_state['values']['dispute_factor_details']);
  
  $cache->ewrapper->save();
}

/**
* Implements hook_form for "agencies"
*/
function referral_agencies_form($form, &$form_state) {
  referral_create_referral();
  if (!isset($form['#parents'])) {
    $form['#parents'] = array();
  }
  $cache = dispute_referral_get_page_cache('case_data');
  $node = node_load($cache->ewrapper->nid->value());
  
  $form['#attached']['js'] = array(
    drupal_get_path('module', 'referral') . '/referral.js',
  );
  
  $items = _fetch_default_value('field_agencies_involved', $cache);
  $instance = field_info_instance('node', 'field_agencies_involved', 'case_management_case');
  $field = field_info_field_by_id($instance['field_id']);
  $form_element = field_default_form('node', $node, $field, $instance, 'und', $items, $form, $form_state);
  $form_element['field_agencies_involved']['#weight'] = '1';
  $form = array_merge($form, $form_element);
  $other_key = array_search('Other', $form_element['field_agencies_involved']['und']['#options']);
  
  $form['other_agency_name'] = array(
    '#type' => 'textfield',
    '#title' => 'Other agency name(s)',
    '#weight' => '2',
    '#states' => array(
      'visible' => array(
        ':input[name="field_agencies_involved[und][' . $other_key .']"]' => array('checked' => TRUE),
	  ),
    ),
    '#default_value' => _fetch_default_value('field_other_agency', $cache, ''),
  );
  
  $form['agency_contact_intro'] = array(
    '#markup' => '<p>If you want us to inform other organisations of the outcome of mediation, please provide names and contact details below.</p>',
    '#weight' => '3',
  );
  
  $items = _fetch_default_value('field_agency_contacts', $cache, array());
  $instance = field_info_instance('node', 'field_agency_contacts', 'case_management_case');
  $field = field_info_field_by_id($instance['field_id']);
  $form_element = field_default_form('node', $node, $field, $instance, 'und', $items, $form, $form_state);
  if (isset($form_element['field_agency_contacts']['und']['form'])) {
    unset($form_element['field_agency_contacts']['und']['form']['status']);
	$form_element['field_agency_contacts']['und']['form']['#title'] = 'Add new contact';
	$form_element['field_agency_contacts']['und']['form']['actions']['ief_add_save']['#value'] = 'Create contact';
	$form_element['field_agency_contacts']['und']['form']['field_residents']['und']['#empty_option'] = '- Select a value -';
  }
  $form_element['field_agency_contacts']['und']['actions']['ief_add']['#value'] = 'Add a contact';
  $form = array_merge($form, $form_element);
  $form['#attached']['js'] = array(
    drupal_get_path('module', 'referral') . '/referral.js',
  );
  $cache->ewrapper->save();
  
  if( !isset( $form_state['triggering_element']['#ajax'] ) && isset($cache->confirm_routed)) {
    // The form shown during the confirmation step:
    $node_values = array(
      'status' => 0,
      'comment' => 0,
      'promote' => 0,
    );
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
  
/**
* Implements hook_form_FORM_ID_alter for "agencies" form
*/
function referral_form_referral_agencies_form_alter (&$form, &$form_state, $form_id) {
  _fix_subform_back_button($form);
  
  // Switch off the control for published/unpublished status
  if (isset($form['field_agency_contacts']['und']['entities']['0']['form']['status'])) {
  	unset($form['field_agency_contacts']['und']['entities']['0']['form']['status']);
  }
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
	    unset($form['field_agencies_involved']);
	    unset($form['other_agency_name']);
	    unset($form['agency_contact_intro']);
	    unset($form['field_agency_contacts']);
	  }
	  if (isset($form['buttons']['return']['#wizard type'])) {
	    $form['buttons']['return']['#wizard type'] = 'return';
	  }
  	}
  $cache->confirm_routed = FALSE;
  dispute_referral_set_page_cache('case_data', $cache);
  }
}

/**
* Implements hook_form_validate for "agencies" form
*/
function referral_agencies_form_validate (&$form, &$form_state) {

}

/**
* Implements hook_form_submit for "agencies" form
*/
function referral_agencies_form_submit (&$form, &$form_state) {
  $form_state['rebuild'] = $form_state['plan_rebuild'];
  
  $cache = dispute_referral_get_page_cache('case_data');
  // Check if submit is being run for the second time (when finish is confirmed)
  if (!isset($cache->agencies_submitted)) {
    $node = node_load($cache->ewrapper->nid->value());
  
    // Update value for agencies involved
    $submit_options = array(
      'field_id' => 'field_agencies_involved',
      'deleted' => FALSE,
      'language' => 'und',
    );
    field_attach_submit('node', $node, $form, $form_state, $submit_options);
  
    // Update other agency
    $cache->ewrapper->field_other_agency->set($form_state['values']['other_agency_name']);
  
    //Update field_agency_contacts
    $submit_options = array(
      'field_id' => 'field_agency_contacts',
      'deleted' => FALSE,
    );
    field_attach_submit('node', $node, $form, $form_state, $submit_options);
  
    /// set flag to prevent extra submit for agencies form
    $cache->agencies_submitted = TRUE;
	
	//save updated case node
    $cache->ewrapper->save();
  }
}

/**
* Implements hook_inline_entity_form_table_fields_alter to change the columns
* displayed in the inline entity table
*/
function referral_inline_entity_form_table_fields_alter(&$fields, $context) {
  switch ($context['field_name']) {
    case 'field_party':
      $fields = array(
        // column 1: field_party_reference e.g. "party 1"
        'field_party_reference' => array(
	      'type' => 'field',
          'label' => '',
	      'weight' => 5,
        ),
        // column 2: party name
        'title' => array(
          'type' => 'property',
          'label' => 'Name',
          'weight' => 10,
        ),
      );
	  break;
    case 'field_agency_contacts':
      $fields = array(
        // column 1: contact name
        'field_name' => array(
	      'type' => 'field',
          'label' => 'Name',
	      'weight' => 5,
        ),
        // column 2: contact organiisation
        'title' => array(
          'type' => 'property',
          'label' => 'Organisation',
          'weight' => 10,
        ),
      );
	break;
  }
}

 /**
 * Helper function to use in hook_form_alter 
 * Disables validation and submit processing for back button . 
 */
function _fix_subform_back_button(&$form) {
  if (isset($form['buttons']['previous'])) {
    $form['buttons']['previous']['#limit_validation_errors'] = array();
	$form['buttons']['previous']['#submit'] = array('ctools_wizard_submit');
	$form['buttons']['previous']['#wizard type'] = 'next';
  }
}

 /**
 * Retrieves the data for a specified field from the case node (if previously set) and 
 * reformats it to use as the default value in the form control. 
 * @field_name - name of the node field
 * @cache - the ctools cache object
 * @null_value - value to return if no data previously saved 
 */
function _fetch_default_value ($field_name, $cache, $null_value = NULL) {
  $info = field_info_field($field_name);
  $node = node_load($cache->ewrapper->nid->value());
  if (isset($node->$field_name)  && !empty($node->$field_name)) {
  	switch($info['type']) {
	  case 'date':
		$return_array = array();
		$element = $node->$field_name;
		$time_string = $element['und'][0]['value'];
		$return_array[] = array(
		  'value' => $time_string,
		);
		return $return_array;
	  case 'taxonomy_term_reference':
	    $terms = $cache->ewrapper->$field_name->value();
		$issues = array();
		foreach ($terms as $key => $term){
		  $issues[] = array('tid' => $term->tid);
		}
		return $issues;
	  case 'entityreference':
		$items = array();
		foreach($cache->ewrapper->$field_name->value() as $id => $party){
		  $items[] = array('target_id' => $party->nid);
		}
		return $items;
	  case 'text_with_summary':
	    return $cache->ewrapper->$field_name->value()['value'];
	  default:
		return $cache->ewrapper->$field_name->value();
	}
  }
  else {
  	return $null_value;
  }
}

/**
* Creates the initial case node, if it doesn't already exist, with initial housekeeping data
* also creates an entity wrapper for the node and saves it with nid value in ctools cache. 
*/
function referral_create_referral(){
  global $user;
  global $language;
  $lang_name = $language->language;
  $u_wrapper = entity_metadata_wrapper('user', $user);
  $cache = dispute_referral_get_page_cache('case_data');
  // Set initial values including bundla name
  if (!isset($cache->nid)) {
    $node_values = array(
      'status' => 0,
      'comment' => 0,
      'promote' => 0,
      'language' => $lang_name,
      'type' => 'case_management_case',
      'uid' => $user->uid,
    );
	$node = entity_create('node', $node_values);
	node_save($node);
	$cache->nid = $node->nid;
	$cache->ewrapper = entity_metadata_wrapper('node', $node);
	
	// create a title by combining organisation name, referrer's last name and a the first
	//six characters of a hash value to give a practically unique reference
	$case_title = $u_wrapper->field_organisation->name->value() . ' - ';
    $case_title .= $u_wrapper->field_last_name->value() . ' - ';
    $case_title .= substr(sha1($u_wrapper->field_first_name->value() . time()), 0, 6);
    $cache->ewrapper->title->set($case_title);
	
	// Set today as the value for referral date 
	$referred_date = new DateTime();
    $node->field_referred_date[LANGUAGE_NONE][0] = array(
      'value' => date_format($referred_date, 'Y-m-d'),
      'timezone' => drupal_get_user_timezone(),
      'timezone_db' => date_default_timezone(),
    );
	
	// Set the status of the case as "referred"
	$case_status_term = taxonomy_get_term_by_name('referred', 'case_status');
    $cache->ewrapper->field_case_status->set(key($case_status_term));
    dispute_referral_set_page_cache('case_data', $cache);
	node_save($node);
  }
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
  }
  
  
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




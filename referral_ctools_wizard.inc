<?php

/**
* Create callback for standard ctools registration wizard.
*/
function dispute_referral_ctools_wizard($step = 'outline') {
  // Include required ctools files.
  ctools_include('wizard');
  ctools_include('object-cache');
   
  $form_info = array(
    // Specify unique form id for this form.
    'id' => 'multistep_dispute_referral',
    //Specify the path for this form. It is important to include space for the $step argument to be passed.
    'path' => "referral/dispute/%step",
    // Show breadcrumb trail.
    'show trail' => TRUE,
    'show back' => FALSE,
    'show return' => FALSE,
    // Callback to use when the 'next' button is clicked.
    'next callback' => 'dispute_referral_subtask_next',
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
    'forms' => array(
      'outline' => array(
        'form id' => 'referral_outline_form'
      ),
      'parties' => array(
        'form id' => 'referral_parties_form',
        // Be sure to load the required include file if the form callback is not defined in the .module file.
        //'include' => drupal_get_path('module', 'grasmash_registration') . '/grasmash_registration_groups_form.inc',
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
  $form_state['referral_object'] = dispute_referral_get_page_cache('dispute_referral');

 
// Return the form as a Ctools multi-step form.
  $output = ctools_wizard_multistep_form($form_info, $step, $form_state);
 
  return $output;
}

/**
* Retreives an object from the cache.
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
function dispute_registration_clear_page_cache($name) {
  ctools_include('object-cache');
  ctools_object_cache_clear('dispute_referral', $name);
}

/**
* Callback executed when the 'next' button is clicked.
*/
function dispute_registration_subtask_next(&$form_state) {
  // Store submitted data in a ctools cache object, namespaced 'referral'.
  dispute_referral_set_page_cache('dispute_referral', $form_state['values']);
}

/**
* Callback executed when the 'cancel' button is clicked.
*/
function dispute_referral_subtask_cancel(&$form_state) {
  // Clear our ctools cache object. It's good housekeeping.
  dispute_referral_clear_page_cache('dispute_referral');
}

/**
* Callback executed when the entire form submission is finished.
*/
function dispute_registration_subtask_finish(&$form_state) {
  // Clear our Ctool cache object.
  dispute_referral_clear_page_cache('dispute_referral');

 
// Redirect the user to the front page.
  drupal_goto('<front>');
}

function referral_outline_form($form, &$form_state) {
  $form['item'] = array(
    '#markup' => t('This is step 1'),
  );

  return $form;
}
function referral_parties_form($form, &$form_state) {
  $form['item'] = array(
    '#markup' => t('This is step 2'),
  );

  return $form;
}

function referral_features_form($form, &$form_state) {
  $form['item'] = array(
    '#markup' => t('This is step 3'),
  );

  return $form;
}

function referral_agencies_form($form, &$form_state) {
  $form['item'] = array(
    '#markup' => t('This is step 4'),
  );

  return $form;
}
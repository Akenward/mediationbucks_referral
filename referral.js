/**
 * @file
 * Provides JavaScript for ...
 */

Drupal.behaviors.referral_remove_tabledrag_toggle = {
  attach: function (context, settings) {
    (function ($) {
    // Clear the 'show row weights' and 'cardinality count' messages
      $('.tabledrag-toggle-weight').text(Drupal. t(''));
      $('.ief-cardinality-count').html('');
      if (Drupal.settings.referral.subformFlag) {
        $('.form-actions').hide()
      }
      else {
      	$('.form-actions').show()
      }
    }(jQuery));
  }
};

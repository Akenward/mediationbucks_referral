/**
 * @file
 * Provides JavaScript for ...
 */

Drupal.behaviors.referral_remove_tabledrag_toggle = {
  attach: function (context, settings) {
    (function ($) {
    // All your code here
      $('.tabledrag-toggle-weight').text(Drupal. t(''));
      $('.ief-cardinality-count').html('');
    }(jQuery));
  }
};

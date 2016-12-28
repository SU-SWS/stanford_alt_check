<?php

/**
 * @file
 * Stanford Alt Check Theme.
 */

/**
 * Themes the form as a table.
 *
 * @param array $vars
 *   Form array.
 *
 * @return string
 *   Rendered html.
 */
function theme_stanford_alt_check_images($vars) {
  $form = $vars['form'];

  $header = array(
    t('Image'),
    t('Entity'),
    t('Alt Attribute'),
  );
  $rows = array();
  foreach (element_children($form['images']) as $field_name) {
    foreach (element_children($form['images'][$field_name]) as $type) {
      foreach (element_children($form['images'][$field_name][$type]) as $entity_id) {
        foreach (element_children($form['images'][$field_name][$type][$entity_id]) as $fid) {
          $rows[] = array(
            drupal_render($form['images'][$field_name][$type][$entity_id][$fid]['image']),
            drupal_render($form['images'][$field_name][$type][$entity_id][$fid]['entity_uri']),
            drupal_render($form['images'][$field_name][$type][$entity_id][$fid]['alt']),
          );
        }
      }
    }
  }
  $current_page = pager_default_initialize(count($rows), 10);
  $chunks = array_chunk($rows, 10, TRUE);

  $output = theme_table(array(
    'header' => $header,
    'rows' => $chunks[$current_page],
    'attributes' => array(),
    'empty' => t('No empty alt attributes at this time.'),
    'caption' => '',
    'colgroups' => array(),
    'sticky' => FALSE,
  ));

  $output .= theme('pager', array('quantity', count($rows)));
  $output .= drupal_render_children($form);

  return $output;
}

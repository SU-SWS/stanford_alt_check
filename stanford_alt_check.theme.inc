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
function theme_stanford_alt_check_image_uploads(array $vars) {
  $form = $vars['form'];
  $header = array(
    array('data' => t('Image')),
    array('data' => t('Entity'), 'field' => 'Entity'),
    array('data' => t('Published'), 'field' => 'Published'),
    array('data' => t('Alt Attribute'), 'field' => 'Alt Attribute'),
    array('data' => t('Last Changed'), 'field' => 'Last Changed'),
  );

  $rows = array();
  foreach (element_children($form['images']) as $field_name) {
    foreach (element_children($form['images'][$field_name]) as $type) {
      foreach (element_children($form['images'][$field_name][$type]) as $entity_id) {
        foreach (element_children($form['images'][$field_name][$type][$entity_id]) as $fid) {
          $rows[] = array(
            'Image' => drupal_render($form['images'][$field_name][$type][$entity_id][$fid]['image']),
            'Entity' => drupal_render($form['images'][$field_name][$type][$entity_id][$fid]['entity_uri']),
            'Published' => drupal_render($form['images'][$field_name][$type][$entity_id][$fid]['entity_status']),

            // Sets two alts for sorting simplicity.
            'Alt Attribute' => $form['images'][$field_name][$type][$entity_id][$fid]['alt']['#default_value'],
            'Alt Form' => drupal_render($form['images'][$field_name][$type][$entity_id][$fid]['alt']),

            // Sets two changed for sorting simplicity.
            'Changed' => drupal_render($form['images'][$field_name][$type][$entity_id][$fid]['changed']),
            'Last Changed' => drupal_render($form['images'][$field_name][$type][$entity_id][$fid]['changed_stamp']),
          );
        }
      }
    }
  }

  // Sort then unset columns used only for sorting.
  usort($rows, 'stanford_alt_check_sort_rows');
  foreach ($rows as &$row) {
    unset($row['Alt Attribute']);
    unset($row['Last Changed']);
  }

  // Pager.
  $num_per_page = 25;
  $current_page = pager_default_initialize(count($rows), $num_per_page);
  $chunks = array_chunk($rows, $num_per_page, TRUE);

  $table_vars = array(
    'header' => $header,
    'rows' => $chunks[$current_page],
    'attributes' => array(),
    'empty' => t('No empty alt attributes at this time.'),
    'caption' => '',
    'colgroups' => array(),
    'sticky' => FALSE,
  );

  $output = theme('table', $table_vars);
  $output .= theme('pager', array('quantity', count($rows)));
  $output .= drupal_render_children($form);

  return $output;
}

/**
 * Themes the form as a table.
 *
 * @param array $vars
 *   Form array.
 *
 * @return string
 *   Rendered html.
 */
function theme_stanford_alt_check_image_wysiwyg(array $vars) {
  $form = $vars['form'];

  $header = array(
    array('data' => t('Image')),
    array('data' => t('Entity'), 'field' => 'Entity'),
    array('data' => t('Published'), 'field' => 'Published'),
    array('data' => t('Alt Attribute'), 'field' => 'Alt Attribute'),
    array('data' => t('Last Changed'), 'field' => 'Last Changed'),
  );

  $rows = array();
  foreach (element_children($form['images']) as $field_name) {
    foreach (element_children($form['images'][$field_name]) as $type) {
      foreach (element_children($form['images'][$field_name][$type]) as $entity_id) {
        foreach (element_children($form['images'][$field_name][$type][$entity_id]) as $delta) {
          foreach (element_children($form['images'][$field_name][$type][$entity_id][$delta]) as $image_position) {

            $rows[] = array(
              'Image' => drupal_render($form['images'][$field_name][$type][$entity_id][$delta][$image_position]['image']),
              'Entity' => drupal_render($form['images'][$field_name][$type][$entity_id][$delta][$image_position]['entity_uri']),
              'Published' => drupal_render($form['images'][$field_name][$type][$entity_id][$delta][$image_position]['entity_status']),

              // Sets two alts for sorting simplicity.
              'Alt Attribute' => $form['images'][$field_name][$type][$entity_id][$delta][$image_position]['alt']['#default_value'],
              'Alt Form' => drupal_render($form['images'][$field_name][$type][$entity_id][$delta][$image_position]['alt']),

              // Sets two changed for sorting simplicity.
              'Changed' => drupal_render($form['images'][$field_name][$type][$entity_id][$delta][$image_position]['changed']),
              'Last Changed' => drupal_render($form['images'][$field_name][$type][$entity_id][$delta][$image_position]['changed_stamp']),
            );
          }
        }
      }
    }
  }

  // Sort then unset columns used only for sorting.
  usort($rows, 'stanford_alt_check_sort_rows');
  foreach ($rows as &$row) {
    unset($row['Alt Attribute']);
    unset($row['Last Changed']);
  }

  // Pager.
  $num_per_page = 25;
  $current_page = pager_default_initialize(count($rows), $num_per_page);
  $chunks = array_chunk($rows, $num_per_page, TRUE);

  $table_vars = array(
    'header' => $header,
    'rows' => $chunks[$current_page],
    'attributes' => array(),
    'empty' => t('No empty alt attributes at this time.'),
    'caption' => '',
    'colgroups' => array(),
    'sticky' => FALSE,
  );

  $output = theme('table', $table_vars);
  $output .= theme('pager', array('quantity', count($rows)));
  $output .= drupal_render_children($form);

  return $output;
}

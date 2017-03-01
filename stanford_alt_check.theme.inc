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
function theme_stanford_alt_check_form(array $vars) {
  $form = $vars['form'];

  $header = array(
    array('data' => t('Image')),
    array('data' => t('Title'), 'field' => 's.host_title', 'sort' => 'ASC'),
    array('data' => t('Published'), 'field' => 's.host_published'),
    array('data' => t('Alt Attribute'), 'field' => 's.alt_text'),
    array('data' => t('Last Changed'), 'field' => 's.host_changed'),
  );

  $rows = array();
  foreach (element_children($form) as $key) {
    // Skips buttons and fieldsets.
    if (!isset($form[$key]['alt'])) {
      continue;
    }

    $rows[] = array(
      'Image' => drupal_render($form[$key]['image']),
      'Title' => drupal_render($form[$key]['title']),
      'Published' => drupal_render($form[$key]['published']),
      'Alt Text' => drupal_render($form[$key]['alt']),
      'Changed' => drupal_render($form[$key]['changed']),
    );
  }

  $table_vars = array(
    'header' => $header,
    'rows' => $rows,
    'attributes' => array(),
    'empty' => t('No images available at this time.'),
    'caption' => '',
    'colgroups' => array(),
    'sticky' => FALSE,
  );

  $output = drupal_render($form['filter_set']);
  $output .= theme('table', $table_vars);
  $output .= drupal_render_children($form);

  return $output;
}

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
    array('data' => t('Title'), 'field' => 'Title'),
    array('data' => t('Published'), 'field' => 'Published'),
    array('data' => t('Alt Attribute'), 'field' => 'Alt Attribute'),
    array('data' => t('Last Changed'), 'field' => 'Last Changed'),
  );

  $rows = array();
  foreach (element_children($form) as $key) {
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
    'empty' => t('No images available at this time.'),
    'caption' => '',
    'colgroups' => array(),
    'sticky' => FALSE,
  );

  $output = render($form['filter_set']);
  $output .= theme('table', $table_vars);
  $output .= theme('pager', array('quantity', count($rows)));
  $output .= drupal_render_children($form);

  return $output;
}

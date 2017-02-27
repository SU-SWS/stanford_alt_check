<?php

/**
 * @file
 * Stanford Alt Check Upload Form.
 */

/**
 * Stanford Alt check edit form.
 */
function stanford_alt_check_form($form, &$form_state) {
  if (module_exists('colorbox')) {
    // Allow colorbox-load class to open the image.
    _colorbox_doheader();
  }

  $form['#tree'] = TRUE;
  $form = _stanford_alt_check_filter_form($form, $form_state);

  list($rows, $total_rows) = _stanford_alt_check_get_images();

  foreach ($rows as $row_num => $row) {
    $form[$row_num] = array(
      'image' => array('#markup' => _stanford_alt_check_build_image($row['uri'])),
      'title' => array('#markup' => l($row['host_title'], $row['image_meta']['host_uri'])),
      'published' => array('#markup' => $row['host_published'] ? t('Yes') : t('No')),
      'changed' => array('#markup' => format_interval(time() - $row['host_changed'])),
      'alt' => array(
        '#type' => 'textfield',
        '#title' => t('Alt Text for !src', array('!src' => $row['uri'])),
        '#default_value' => $row['alt_text'],
        '#title_display' => 'invisible',
        '#maxlength' => 512,
      ),
    );
    $form_state['images'][$row_num] = $row;
  }

  $form['pager'] = array(
    '#markup' => theme('pager', array(
      'quantity',
      $total_rows,
    )),
  );
  $form['save'] = array('#type' => 'submit', '#value' => t('Save'));
  $form['rescan'] = array(
    '#type' => 'submit',
    '#value' => t('Rescan images'),
    '#submit' => array('_stanford_alt_check_rescan'),
  );
  $form['#attached']['css'][] = drupal_get_path('module', 'stanford_alt_check') . '/css/stanford_alt_check.admin.css';

  return $form;
}

/**
 * Save form submit.
 */
function stanford_alt_check_form_submit($form, &$form_state) {
  $num_changed = 0;
  foreach ($form_state['images'] as $key => $image_data) {
    if ($form_state['values'][$key]['alt'] != $image_data['alt_text']) {
      $num_changed++;
      $entity = entity_load_single($image_data['entity_type'], $image_data['entity_id']);

      if ($image_data['entity_field_wysiwyg']) {
        _stanford_alt_check_wysiwyg_set($entity, $form_state['values'][$key]['alt'], $image_data);
      }
      else {
        $field = $image_data['entity_field'];
        $delta = $image_data['image_meta']['delta'];
        $entity->{$field}[LANGUAGE_NONE][$delta]['alt'] = $form_state['values'][$key]['alt'];
      }
      entity_save($image_data['entity_type'], $entity);
    }
  }
  drupal_set_message(t('Alt text updated for %num images.', array('%num' => $num_changed)));
}

/**
 * Add filter set to the form.
 */
function _stanford_alt_check_filter_form($form, &$form_state) {
  $get = array('alt_text' => 0, 'fields' => 0);
  $get = array_merge($get, drupal_get_query_parameters($_GET));

  // Filters.
  $form['filter_set'] = array(
    '#type' => 'fieldset',
    '#title' => t('Filter'),
  );
  $form['filter_set']['alt_text'] = array(
    '#type' => 'select',
    '#title' => t('Images'),
    '#options' => array(
      0 => t('All'),
      'empty' => t('Empty Alt'),
      'filled' => t('Populated Alt'),
    ),
    '#default_value' => $get['alt_text'],
  );
  $form['filter_set']['fields'] = array(
    '#type' => 'select',
    '#title' => t('Fields'),
    '#options' => array(
      0 => t('All'),
      'wysiwyg' => t('WYSIWYG Fields'),
      'upload' => t('Upload Fields'),
    ),
    '#default_value' => $get['fields'],
  );
  // Filter Buttons.
  $form['filter_set']['actions'] = array('#type' => 'actions');
  $form['filter_set']['actions']['filter_submit'] = array(
    '#type' => 'submit',
    '#value' => t('Filter'),
    '#name' => 'filter-submit',
    '#submit' => array('_stanford_alt_check_form_filter_submit'),
  );
  $form['filter_set']['actions']['filter_reset'] = array(
    '#type' => 'submit',
    '#value' => t('Reset'),
    '#name' => 'filter-reset',
    '#submit' => array('_stanford_alt_check_form_filter_submit'),
  );
  return $form;
}

/**
 * Create the html for the linked image.
 *
 * @param string $uri
 *   Image uri.
 *
 * @return string
 *   The html of the image link.
 */
function _stanford_alt_check_build_image($uri) {
  $thumbnail = $original = $uri;
  $uri_parts = parse_url($uri);
  if (count($uri_parts) <= 2) {
    $thumbnail = image_style_url('thumbnail', $uri);
    $original = file_create_url($uri);
  }

  $image_tag = theme('html_tag', array(
    'element' => array(
      '#tag' => 'img',
      '#attributes' => array(
        'src' => $thumbnail,
        'class' => 'alt-check-image',
      ),
    ),
  ));

  $link_tag = theme('html_tag', array(
    'element' => array(
      '#tag' => 'a',
      '#value' => $image_tag,
      '#attributes' => array(
        'href' => $original,
        'class' => 'colorbox-load',
        'target' => '_blank',
      ),
    ),
  ));

  return $link_tag;
}

/**
 * Filter form submit.
 */
function _stanford_alt_check_form_filter_submit($form, &$form_state) {
  if ($form_state['clicked_button']['#name'] == 'filter-reset') {
    $form_state['redirect'] = current_path();
    return;
  }

  $get = drupal_get_query_parameters($_GET);
  if ($form_state['values']['filter_set']['alt_text']) {
    $get['alt_text'] = $form_state['values']['filter_set']['alt_text'];
  }
  else {
    unset($get['alt_text']);
  }

  if ($form_state['values']['filter_set']['fields']) {
    $get['fields'] = $form_state['values']['filter_set']['fields'];
  }
  else {
    unset($get['fields']);
  }

  $form_state['redirect'] = array(
    current_path(),
    array('query' => array($get)),
  );
}

/**
 * Sets the alt text in the entity using the values from $value.
 *
 * @param object $entity
 *   The main object with the field data.
 * @param string $alt
 *   The alt to set.
 * @param array $image_data
 *   The submitted data.
 */
function _stanford_alt_check_wysiwyg_set($entity, $alt, array $image_data) {
  $field = $image_data['entity_field'];
  $delta = $image_data['image_meta']['delta'];
  $field_items = field_get_items($image_data['entity_type'], $entity, $field);
  $field_item = $field_items[$delta];

  $dom = new DOMDocument();
  libxml_use_internal_errors(TRUE);
  $dom->loadHTML($field_item['value'], LIBXML_HTML_NOIMPLIED | LIBXML_HTML_NODEFDTD);
  /** @var DOMElement $image */
  foreach ($dom->getElementsByTagName('img') as $position => $image) {
    if (
      $position == $image_data['image_meta']['position'] &&
      $image->getAttribute('src') == $image_data['image_meta']['raw_url']
    ) {
      $image->setAttribute('alt', $alt);
      break;
    }
  }
  $field_item['value'] = $dom->saveHTML();

  $entity->{$field}[LANGUAGE_NONE][$delta] = $field_item;
}

/**
 * Find all fieldable entities and create a batch to rescan them.
 */
function _stanford_alt_check_rescan() {
  db_truncate('stanford_alt_check')->execute();

  $batch = array(
    'operations' => array(),
    'title' => t('Rescan Entities'),
    'init_message' => t('Starting to rescan...'),
    'progress_message' => t('Scanned @current out of @total.'),
    'error_message' => t('Unable to rescan all entities.'),
  );

  foreach (entity_get_info() as $type => $definition) {
    if ($definition['fieldable']) {

      // Use EFQ so we dont have to use entity_load() and run out of memory.
      $field_query = new EntityFieldQuery();
      $field_query->entityCondition('entity_type', $type);
      $results = $field_query->execute();

      foreach (array_keys($results[$type]) as $entity_id) {
        $batch['operations'][] = array(
          '_stanford_alt_check_rescan_entity',
          array($entity_id, $type),
        );
      }
    }
  }

  batch_set($batch);
  batch_process(current_path());
}

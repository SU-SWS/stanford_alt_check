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

  $get = drupal_get_query_parameters();
  $filter = NULL;
  if (isset($get['filter'])) {
    $filter = $get['filter'];
  }
  $fields = NULL;
  if (isset($get['fields'])) {
    $fields = $get['fields'];
  }

  $form['#tree'] = TRUE;
  $form = _stanford_alt_check_filter_form($form, $form_state, $filter, $fields);

  $rows = array();
  // Exclude rows if filters are set.
  if ($fields != 'wysiwyg') {
    _stanford_alt_check_upload_rows($rows, $filter);
  }
  if ($fields != 'upload') {
    _stanford_alt_check_wysiwgy_rows($rows, $filter);
  }

  // Cleanup.
  $pager = _stanford_alt_check_cleanup_rows($rows);

  foreach ($rows as $row_num => $row) {
    if ($row['type'] == 'upload') {
      // For upload field, there is no position since they have deltas.
      $row['position'] = 0;
    }

    $form[$row_num] = array(
      'image' => array('#markup' => _stanford_alt_check_build_image($row['Image'], $row['original_src'])),
      'title' => array('#markup' => $row['Title']),
      'published' => array('#markup' => $row['Published']),
      'changed' => array('#markup' => format_interval($row['Last Changed'])),
      'alt' => array(
        '#type' => 'textfield',
        '#title' => t('Alt Text for !src', array('!src' => $row['Image'])),
        '#default_value' => $row['Alt Attribute'],
        '#title_display' => 'invisible',
        '#maxlength' => 512,
      ),
    );
    // Sets all the data in form_state for easy access on the submit.
    $form_state['images'][$row_num] = $row;
  }

  $form['pager'] = array('#markup' => $pager);
  $form['save'] = array('#type' => 'submit', '#value' => t('Save'));
  $form['#attached']['css'][] = drupal_get_path('module', 'stanford_alt_check') . '/css/stanford_alt_check.admin.css';

  return $form;
}

/**
 * Add filter set to the form.
 */
function _stanford_alt_check_filter_form($form, &$form_state, $filter, $fields) {
  // Filters.
  $form['filter_set'] = array(
    '#type' => 'fieldset',
    '#title' => t('Filter'),
  );
  $form['filter_set']['filter'] = array(
    '#type' => 'select',
    '#title' => t('Images'),
    '#options' => array(
      0 => t('All'),
      'empty' => t('Empty Alt'),
      'filled' => t('Populated Alt'),
    ),
    '#default_value' => $filter,
  );
  $form['filter_set']['fields'] = array(
    '#type' => 'select',
    '#title' => t('Fields'),
    '#options' => array(
      0 => t('All'),
      'wysiwyg' => t('WYSIWYG Fields'),
      'upload' => t('Upload Fields'),
    ),
    '#default_value' => $fields,
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
 * Cleanup, sort and slice the rows so we use what we need.
 *
 * @param array $rows
 *   All rows to be cleaned up.
 *
 * @return string
 *   Themed pager.
 */
function _stanford_alt_check_cleanup_rows(array &$rows) {
  foreach ($rows as &$row) {
    if (is_null($row['Alt Attribute'])) {
      $row['Alt Attribute'] = '';
    }
  }

  // Sort rows.
  usort($rows, 'stanford_alt_check_sort_rows');

  // Create pager and build pager chunks.
  $total_rows = count($rows);
  $num_per_page = 10;
  $current_page = pager_default_initialize(count($rows), $num_per_page);
  $chunks = array_chunk($rows, $num_per_page, TRUE);
  $rows = $chunks[$current_page];

  return theme('pager', array('quantity', $total_rows));
}

/**
 * Create the html for the linked image.
 *
 * @param string $thumbnail
 *   Image style url.
 * @param string $source
 *   Original image url.
 *
 * @return string
 *   The html of the image link.
 */
function _stanford_alt_check_build_image($thumbnail, $source) {
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
        'href' => $source,
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
  if ($form_state['values']['filter_set']['filter']) {
    $get['filter'] = $form_state['values']['filter_set']['filter'];
  }
  else {
    unset($get['filter']);
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
 * Save form submit.
 */
function stanford_alt_check_form_submit($form, &$form_state) {
  $num_changed = 0;
  foreach ($form_state['images'] as $key => $image_data) {
    if ($form_state['values'][$key]['alt'] != $image_data['Alt Attribute']) {
      $num_changed++;
      $entity = entity_load_single($image_data['entity_info']['entity_type'], $image_data['entity_info']['entity_id']);
      if ($image_data['type'] == 'wysiwyg') {
        _stanford_alt_check_wysiwyg_set($entity, $form_state['values'][$key]['alt'], $image_data);
      }
      else {
        $field = $image_data['entity_info']['field_name'];
        $delta = $image_data['delta'];
        $entity->{$field}[LANGUAGE_NONE][$delta]['alt'] = $form_state['values'][$key]['alt'];
      }
      entity_save($image_data['entity_info']['entity_type'], $entity);
    }
  }
  drupal_set_message(t('Alt text updated for %num images.', array('%num' => $num_changed)));
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
  $field = $image_data['entity_info']['field_name'];
  $field_items = field_get_items($image_data['entity_info']['entity_type'], $entity, $field);
  $field_item = $field_items[$image_data['delta']];
  $dom = new DOMDocument();
  libxml_use_internal_errors(TRUE);
  $dom->loadHTML($field_item['value'], LIBXML_HTML_NOIMPLIED | LIBXML_HTML_NODEFDTD);
  /** @var DOMElement $image */
  foreach ($dom->getElementsByTagName('img') as $position => $image) {
    if ($position == $image_data['position'] && $image->getAttribute('src') == $image_data['raw_url']) {
      $image->setAttribute('alt', $alt);
      break;
    }
  }
  $field_item['value'] = $dom->saveHTML();
  $entity->{$field}[LANGUAGE_NONE][$image_data['delta']] = $field_item;
}

/**
 * Build the rows for the table from the data for text fields.
 *
 * @param array $rows
 *   Any existing rows to add to.
 * @param string|null $filter
 *   If the data should be filtered.
 *   Possible values:
 *     'filled': images with populated alts.
 *     'empty': images with empty alts.
 */
function _stanford_alt_check_wysiwgy_rows(&$rows = array(), $filter = NULL) {
  foreach (_stanford_alt_check_wysiwgy_fields() as $field_name) {
    $field_query = new EntityFieldQuery();
    $field_query->fieldCondition($field_name, 'value', '%<img%', 'like');
    $results = $field_query->execute();

    foreach ($results as $entity_type => $entities) {
      $entities = entity_load($entity_type, array_keys($entities));

      foreach ($entities as $entity_id => $entity) {
        $host_entity_info = stanford_alt_check_get_host($entity, $entity_type);

        // Cant find host entity info.
        if (!$host_entity_info) {
          continue;
        }

        list($host_entity, $host_type, $host_id, $title, $host_url, $published) = $host_entity_info;
        if (!($field_items = field_get_items($entity_type, $entity, $field_name))) {
          continue;
        }

        foreach ($field_items as $delta => $field_item) {
          // Create a raw images array to obtain the raw src attribute later.
          $dom = new DOMDocument();
          libxml_use_internal_errors(TRUE);
          $dom->loadHTML($field_item['value']);
          $raw_images = $dom->getElementsByTagName('img');

          $dom = new DOMDocument();
          $dom->loadHTML(check_markup($field_item['value'], $field_item['format']));
          /** @var DOMElement $image */
          foreach ($dom->getElementsByTagName('img') as $position => $image) {
            $wysiwyg_item = _stanford_alt_check_build_wysiwyg_item($image, $filter);
            if (count(array_filter($wysiwyg_item)) < 2) {
              continue;
            }
            list($image_url, $original_url) = $wysiwyg_item;


            $rows[] = array(
              'Image' => $image_url,
              'Title' => l(t($title), $host_url),
              'Published' => $published,
              'Alt Attribute' => $image->getAttribute('alt'),
              'Last Changed' => time() - $host_entity->changed,
              'type' => 'wysiwyg',
              'delta' => $delta,
              'position' => $position,
              'original_src' => $original_url,
              'raw_url' => $raw_images[$position]->getAttribute('src'),
              'entity_info' => array(
                'entity_type' => $entity_type,
                'entity_id' => $entity_id,
                'field_name' => $field_name,
                'host_type' => $host_type,
                'host_id' => $host_id,
                'host_url' => $host_url,
              ),
            );
          }
        }
      }
    }
  }
}

/**
 * Parse the DOMElement to get the paths of a thumbnail and original image.
 *
 * @param DOMElement $image
 *   The image element from the html.
 * @param string $filter
 *   The filter.
 *
 * @return array
 *   A keyed array of original image and the style image.
 */
function _stanford_alt_check_build_wysiwyg_item(DOMElement $image, $filter) {
  $alt = $image->getAttribute('alt');
  $src = $image->getAttribute('src');
  if (($filter == 'empty' && $alt) || ($filter == 'filled' && !$alt)) {
    return array();
  }

  $src_parts = parse_url($src);
  $original_url = $image_url = $src;
  global $base_url;
  if (!isset($src_parts['host']) || strpos($base_url, $src_parts['host']) !== FALSE) {
    if (strpos($src_parts['path'], '/styles/') !== FALSE) {
      // Image derivative.
      $exploded = explode('/', $src_parts['path']);
      $style_pos = array_search('styles', $exploded) + 3;
      $src = '//' . implode('/', array_slice($exploded, $style_pos));
    }
    else {
      // Original Image.
      $src = str_replace(variable_get('file_public_path'), '', $src_parts['path']);
      $src = str_replace(variable_get('file_private_path'), '', $src);
      $src = '//' . trim($src, '/');
    }
  }

  $fid = db_select('file_managed', 'f')
    ->fields('f', array('fid'))
    ->condition('uri', "%$src", 'like')
    ->execute()
    ->fetchField();

  if ($fid) {
    $file = file_load($fid);
    $image_url = image_style_url('thumbnail', $file->uri);
    $original_url = file_create_url($file->uri);
  }
  return array($image_url, $original_url);
}

/**
 * Get all fields with possible html.
 *
 * @return array
 *   All fields with text_processing configured.
 */
function _stanford_alt_check_wysiwgy_fields() {
  $fields = array();
  $wywisyg_field_types = array('text', 'text_long', 'text_with_summary');
  foreach (field_info_field_map() as $field_name => $field) {
    if (in_array($field['type'], $wywisyg_field_types)) {
      foreach ($field['bundles'] as $type => $bundles) {
        foreach ($bundles as $bundle) {
          $instance = field_info_instance($type, $field_name, $bundle);

          // HTML is available.
          if ($instance['settings']['text_processing']) {
            $fields[$field_name] = $field_name;
          }
        }
      }
    }
  }
  return $fields;
}

/**
 * Build the rows for the table from the data for image fields.
 *
 * @param array $rows
 *   Any existing rows to add to.
 * @param string|null $filter
 *   If the data should be filtered.
 *   Possible values:
 *     'filled': images with populated alts.
 *     'empty': images with empty alts.
 */
function _stanford_alt_check_upload_rows(&$rows = array(), $filter = NULL) {
  foreach (_stanford_alt_check_upload_fields() as $field) {
    $results = _stanford_alt_check_get_results($field['field_name'], $filter);
    foreach ($results as $entity_type => $entities) {
      $entities = entity_load($entity_type, array_keys($entities));

      foreach ($entities as $entity_id => $entity) {

        $host_entity_info = stanford_alt_check_get_host($entity, $entity_type);
        if (!$host_entity_info) {
          continue;
        }

        list($host_entity, $host_type, $host_id, $title, $host_url, $published) = $host_entity_info;

        $field_items = field_get_items($entity_type, $entity, $field['field_name']);

        foreach ($field_items as $delta => $item) {
          $rows[] = array(
            'Image' => image_style_url('thumbnail', $item['uri']),
            'Title' => l(t($title), $host_url),
            'Published' => $published,
            'Alt Attribute' => $item['alt'],
            'Last Changed' => time() - $host_entity->changed,
            'type' => 'upload',
            'delta' => $delta,
            'original_src' => file_create_url($item['uri']),
            'entity_info' => array(
              'entity_type' => $entity_type,
              'entity_id' => $entity_id,
              'field_name' => $field['field_name'],
              'host_type' => $host_type,
              'host_id' => $host_id,
              'host_url' => $host_url,
            ),
          );
        }
      }
    }
  }
}

/**
 * Use the field and filter to do entity field queries and obtain a list.
 *
 * @param string $field
 *   The field name on the entity.
 * @param string|null $filter
 *   If we want to filter out certain entities.
 *
 * @return array
 *   The keyed array provided by EntityFieldQuery.
 */
function _stanford_alt_check_get_results($field, $filter = NULL) {
  if ($filter == 'empty') {
    // Get filled alt attributes to negate.
    $filled_results = _stanford_alt_check_get_results($field, 'filled');

    if (!$filled_results) {
      // If no entities have filled in alt attributes, just give all entities
      // with images.
      return _stanford_alt_check_get_results($field);
    }

    $empty_results = array();
    foreach (array_keys($filled_results) as $type) {

      // Get all entities that do not have populated alt attributes.
      $field_query = new EntityFieldQuery();
      $field_query->entityCondition('entity_type', $type)
        ->entityCondition('entity_id', array_keys($filled_results[$type]), 'NOT IN')
        ->fieldCondition($field, 'fid', '0', '>=');

      // Merge the results together.
      foreach ($field_query->execute() as $type => $entities) {
        if (!isset($empty_results[$type])) {
          $empty_results[$type] = $entities;
        }
        else {
          $empty_results[$type] = array_merge($empty_results[$type], $entities);
        }
      }
    }

    return $empty_results;
  }
  elseif ($filter == 'filled') {
    // Only entites with populated image alt attributes.
    $field_query = new EntityFieldQuery();
    $field_query->fieldCondition($field, 'fid', '0', '>=');
    $field_query->fieldCondition($field, 'alt', '', '!=');
    return $field_query->execute();
  }

  // All entities with images.
  $field_query = new EntityFieldQuery();
  $field_query->fieldCondition($field, 'fid', '0', '>=');
  return $field_query->execute();
}

/**
 * Get all fields of image type.
 *
 * @return array
 *   All image fields.
 */
function _stanford_alt_check_upload_fields() {
  $fields = array();
  foreach (field_info_fields() as $field) {
    if ($field['type'] == 'image') {
      $fields[] = $field;
    }
  }
  return $fields;
}

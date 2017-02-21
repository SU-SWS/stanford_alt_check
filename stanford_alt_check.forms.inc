<?php

/**
 * @file
 * Stanford Alt Check Upload Form.
 */

/**
 * Alt Check Images form callback.
 */
function stanford_alt_check_image_uploads($form, &$form_state) {
  if (module_exists('colorbox')) {
    _colorbox_doheader();
  }

  drupal_set_title(t("Image Uploads"));
  $form = array();
  $form['#tree'] = TRUE;

  $filter = '0';
  $get = drupal_get_query_parameters($_GET);
  if (isset($get['filter'])) {
    $filter = $get['filter'];
  }
  $form += stanford_alt_check_filter_form($filter);

  $image_entities = stanford_alt_check_upload_entities($filter);
  foreach ($image_entities as $field => $types) {
    foreach ($types as $type => $bundles) {
      foreach ($bundles as $entities) {
        foreach ($entities as $id => $entity) {
          if ($images = field_get_items($type, $entity, $field)) {
            foreach ($images as $image) {
              $image_row = stanford_alt_check_get_image_form_component($image, $entity, $type);
              if ($image_row) {
                $form['images'][$field][$type][$id][$image['fid']] = $image_row;
              }
            }
          }
        }
      }
    }
  }
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );
  $form['#attached']['css'][] = drupal_get_path('module', 'stanford_alt_check') . '/css/stanford_alt_check.admin.css';
  return $form;
}

/**
 * Alt check images submit handler to save entities with new alt attributes.
 */
function stanford_alt_check_image_uploads_submit($form, &$form_state) {
  $changed = 0;
  foreach ($form_state['values']['images'] as $field_name => $types) {
    foreach ($types as $type => $entity_ids) {
      foreach ($entity_ids as $entity_id => $files) {
        foreach ($files as $fid => $alt) {
          if ($form['images'][$field_name][$type][$entity_id][$fid]['alt']['#default_value'] !== $alt['alt']) {
            $entities_array = entity_load($type, array($entity_id));
            $entity = reset($entities_array);
            $field_items = &$entity->{$field_name}[LANGUAGE_NONE];
            foreach ($field_items as &$item) {
              if ($item['fid'] == $fid) {
                $item['alt'] = $alt['alt'];
              }
            }
            entity_save($type, $entity);
            $changed++;
          }
        }
      }
    }
  }

  drupal_set_message(t('Alt Attributes updated for !num images', array('!num' => $changed)));
}

/**
 * Menu callback to display list of entities with missing alt attributes.
 */
function stanford_alt_check_image_wysiwyg($form, &$form_state) {
  if (module_exists('colorbox')) {
    _colorbox_doheader();
  }

  drupal_set_title(t("WYSIWYG Images"));
  $form = array();
  $form['#tree'] = TRUE;

  $filter = '0';
  $get = drupal_get_query_parameters($_GET);
  if (isset($get['filter'])) {
    $filter = $get['filter'];
  }
  $form += stanford_alt_check_filter_form($filter);

  $image_entities = stanford_alt_check_get_wysiwyg($filter);
  foreach ($image_entities as $field => $types) {
    foreach ($types as $type => $bundles) {
      foreach ($bundles as $entities) {
        foreach ($entities as $id => $field_items) {
          foreach ($field_items as $delta => $images) {
            $entity = entity_load($type, array($id));
            $entity = reset($entity);
            foreach ($images as $position => $image) {
              $image_row = stanford_alt_check_get_image_form_component($image, $entity, $type);
              if ($image_row) {
                $form['images'][$field][$type][$id][$delta][$position] = $image_row;
              }
            }
          }
        }
      }
    }
  }
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
  );
  $form['#attached']['css'][] = drupal_get_path('module', 'stanford_alt_check') . '/css/stanford_alt_check.admin.css';
  return $form;
}

/**
 * Save the image new alt text.
 */
function stanford_alt_check_image_wysiwyg_submit($form, &$form_state) {
  $changed = 0;
  foreach ($form_state['values']['images'] as $field_name => $types) {
    foreach ($types as $type => $entities) {
      foreach ($entities as $id => $field_items) {
        $entity = NULL;
        foreach ($field_items as $delta => $images) {
          foreach ($images as $position => $alt) {

            // No alt provided.
            if (!$alt['alt']) {
              continue;
            }

            // Current alt matches existing alt.
            if ($form['images'][$field_name][$type][$id][$delta][$position]['alt']['#default_value'] == $alt['alt']) {
              continue;
            }

            $changed++;
            $entity = entity_load($type, array($id));
            $entity = reset($entity);

            stanford_alt_check_set_wysiwyg_alt($entity->{$field_name}[LANGUAGE_NONE][$delta], $position, $alt['alt']);
          }

          if ($entity) {
            entity_save($type, $entity);
          }
        }
      }
    }
  }

  drupal_set_message(t('Alt Attributes updated for !num images', array('!num' => $changed)));
}

/**
 * Use DOMDocument to find the image and set the alt text.
 *
 * @param array $field
 *   Field item data.
 * @param int $position
 *   The position of the image in the html as found by
 *   DOMDocument->getElementsByTagName('img').
 * @param string $alt
 *   The alt attribut string to set.
 */
function stanford_alt_check_set_wysiwyg_alt(array &$field, $position, $alt) {
  $dom = new DOMDocument();

  $dom->loadHTML($field['value'], LIBXML_HTML_NOIMPLIED | LIBXML_HTML_NODEFDTD);
  /** @var DOMElement $img_element */
  foreach ($dom->getElementsByTagName('img') as $pos => $img_element) {
    if ($position != $pos) {
      continue;
    }
    $img_element->setAttribute('alt', $alt);
  }
  $field['value'] = $dom->saveHTML();

  if (isset($field['safe_value'])) {
    $dom->loadHTML($field['safe_value'], LIBXML_HTML_NOIMPLIED | LIBXML_HTML_NODEFDTD);
    /** @var DOMElement $img_element */
    foreach ($dom->getElementsByTagName('img') as $pos => $img_element) {
      if ($position != $pos) {
        continue;
      }
      $img_element->setAttribute('alt', $alt);
    }
    $field['safe_value'] = $dom->saveHTML();
  }
}

/**
 * Process the image array to build the form components necessary.
 *
 * @param array $image
 *   Field file array.
 * @param object $entity
 *   Entity Object.
 * @param string $type
 *   Entity type.
 *
 * @return array|bool
 *   Form elements or FALSE if can't get the host entity.
 */
function stanford_alt_check_get_image_form_component(array $image, $entity, $type) {
  // Display the image for quick reference.
  if ($image['external']) {
    $image_url = file_create_url($image['uri']);
  }
  else {
    $image_url = image_style_url('thumbnail', $image['uri']);
  }
  $image_full_url = file_create_url($image['uri']);

  $image_row = array();
  $image_row['image'] = array(
    '#markup' => "<a class=\"colorbox-load\" href='$image_full_url' target='_blank'><img class=\"alt-check-image\" src='$image_url' /></a>",
  );

  if (!($item = stanford_alt_check_get_host($entity, $type))) {
    return FALSE;
  }
  list($host_entity, $host_type) = $item;

  $title = $host_entity->title;
  // Beans use the label property as the identifier.
  if ($host_type == 'bean') {
    $title = $host_entity->label;
  }

  if (strlen($title) > 50) {
    $title = substr($title, 0, 50) . "...";
  }

  // Check if user has access to update the entity displayed.
  if (entity_access('update', $host_type)) {
    $image_row['alt'] = array(
      '#type' => 'textfield',
      '#title' => t('Alt Text for %filename', array('%filename' => $image['filename'])),
      '#default_value' => $image['alt'] ? $image['alt'] : '',
    );
  }
  else {
    $image_row['alt'] = array(
      '#markup' => t('Permission Not Allowed.'),
    );
  }

  $entity_uri = entity_uri($host_type, $host_entity);
  $image_row['entity_uri'] = array(
    '#markup' => l(t($title), $entity_uri['path']),
  );

  // Published or not.
  if (isset($host_entity->status)) {
    $image_row['entity_status'] = array(
      '#markup' => $host_entity->status ? t('Yes') : t('No'),
    );
  }
  else {
    $image_row['entity_status'] = array(
      '#markup' => t('Yes'),
    );
  }

  // Last changed interval.
  $image_row['changed'] = array(
    '#markup' => t('!interval Ago', array('!interval' => format_interval(time() - $host_entity->changed))),
  );
  $image_row['changed_stamp'] = array(
    '#markup' => $host_entity->changed,
  );

  return $image_row;
}

/**
 * Build the portion of the form for the filter.
 *
 * @param string $filter
 *   Default value for the filter select list.
 *
 * @return array
 *   Form components for the filter.
 */
function stanford_alt_check_filter_form($filter) {
  $form = array();
  $form['filter_form'] = array(
    '#type' => 'fieldset',
    '#title' => t('Filter'),
  );
  $form['filter_form']['filter'] = array(
    '#type' => 'select',
    '#title' => t('Images'),
    '#options' => array(
      '0' => t('All'),
      'empty' => t('Images with empty alt'),
      'filled' => t('Images with filled in alt'),
    ),
    '#default_value' => $filter,
  );
  $form['filter_form']['actions'] = array(
    '#type' => 'actions',
  );
  $form['filter_form']['actions']['filter_submit'] = array(
    '#type' => 'submit',
    '#value' => t('Filter'),
    '#name' => 'filter',
    '#submit' => array('stanford_alt_check_form_filter'),
  );
  $form['filter_form']['actions']['filter_reset'] = array(
    '#type' => 'submit',
    '#value' => t('Reset'),
    '#name' => 'reset',
    '#submit' => array('stanford_alt_check_form_filter'),
  );
  return $form;
}

/**
 * Filter form submit.
 */
function stanford_alt_check_form_filter($form, &$form_state) {
  $get = drupal_get_query_parameters($_GET);
  // If user click "Filter" and filter value is applicable.
  if ($form_state['clicked_button']['#name'] == 'filter' && $form_state['values']['filter_form']['filter']) {
    $get['filter'] = $form_state['values']['filter_form']['filter'];
    $form_state['redirect'] = array(current_path(), array('query' => $get));
  }
  else {
    unset($get['filter']);
    $form_state['redirect'] = array(current_path(), array('query' => $get));
  }
}

<?php

/**
 * @file
 * Stanford Alt Check Forms.
 */

/**
 * Alt Check Images form callback.
 *
 * @param array $form
 *   Form.
 * @param array $form_state
 *   Form state.
 *
 * @return array
 *   Renderable form array.
 */
function stanford_alt_check_image_uploads(array $form, array &$form_state) {
  drupal_set_title(t("Image Uploads"));
  $form = array();
  $form['#tree'] = TRUE;

  $image_entities = stanford_alt_check_upload_entities();
  foreach ($image_entities as $field => $types) {
    foreach ($types as $type => $bundles) {
      foreach ($bundles as $entities) {
        foreach ($entities as $id => $entity) {
          if ($images = field_get_items($type, $entity, $field)) {
            foreach ($images as $image) {
              $image_row = stanford_alt_check_get_image_form($image, $entity, $type);
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
  return $form;
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
function stanford_alt_check_get_image_form(array $image, $entity, $type) {
  // Display the image for quick reference.
  $image_url = image_style_url('thumbnail', $image['uri']);
  $image_full_url = file_create_url($image['uri']);

  $image_row = array();
  $image_row['image'] = array(
    '#markup' => "<a href='$image_full_url' target='_blank'><img src='$image_url' /></a>",
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
 * Alt check images submit handler to save entities with new alt attributes.
 *
 * @param array $form
 *   Form array.
 * @param array $form_state
 *   Form state array.
 */
function stanford_alt_check_image_uploads_submit(array $form, array &$form_state) {
  $num_changed = 0;
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
            $num_changed++;
          }
        }
      }
    }
  }

  drupal_set_message(t('Alt Attributes updated for !num images', array('!num' => $num_changed)));
}

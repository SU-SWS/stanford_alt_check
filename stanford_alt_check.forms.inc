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
function stanford_alt_check_images($form, &$form_state) {
  drupal_set_title(t("Image Alt's"));
  $form = array();
  $form['#tree'] = TRUE;

  $image_entities = stanford_alt_check_get_images();
  foreach ($image_entities as $field => $types) {
    foreach ($types as $type => $entities) {
      foreach ($entities as $id => $entity) {
        if ($images = field_get_items($type, $entity, $field)) {
          foreach ($images as $image) {
            if ($image['alt']) {
              continue;
            }
            $image_url = image_style_url('thumbnail', $image['uri']);
            $form['images'][$field][$type][$id][$image['fid']]['image'] = array(
              '#markup' => "<img src='$image_url' />",
            );

            // Check if user has access to update the entity displayed.
            if (entity_access('update', $type)) {
              $form['images'][$field][$type][$id][$image['fid']]['alt'] = array(
                '#type' => 'textfield',
                '#title' => t('Alt Text for %filename', array('%filename' => $image['filename'])),
              );
            }
            else {
              $form['images'][$field][$type][$id][$image['fid']]['alt'] = array(
                '#markup' => t('Permission Not Allowed.'),
              );
            }

            // Field Collection entities.
            if (method_exists($entity, 'hostEntity')) {
              $t = $entity->hostEntityType();
              $e = $entity->hostEntity();
            }
            else {
              $t = $type;
              $e = $entity;
            }

            $entity_uri = entity_uri($t, $e);
            if ($t == 'bean') {
              $title = $e->label;
            }
            else {
              $title = $e->title;
            }


            if (strlen($title) > 50) {
              $title = substr($title, 0, 50) . "...";
            }
            $form['images'][$field][$type][$id][$image['fid']]['entity_uri'] = array(
              '#markup' => l(t($title), $entity_uri['path']),
            );
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
 * Alt check images submit handler to save entities.
 *
 * @param array $form
 *   Form array.
 * @param array $form_state
 *   Form state array.
 */
function stanford_alt_check_images_submit($form, &$form_state) {
  $i = 0;
  foreach ($form_state['values']['images'] as $field_name => $types) {
    foreach ($types as $type => $entity_ids) {
      foreach ($entity_ids as $entity_id => $files) {
        foreach ($files as $fid => $alt) {
          if ($alt['alt']) {
            $entity = reset(entity_load($type, array($entity_id)));
            $field_items = &$entity->{$field_name}[LANGUAGE_NONE];
            foreach ($field_items as &$item) {
              if ($item['fid'] == $fid) {
                $item['alt'] = $alt['alt'];
              }
            }
            entity_save($type, $entity);
            $i++;
          }
        }
      }
    }
  }

  drupal_set_message(t('Alt Attributes saved for !num images', array('!num' => $i)));
}


/**
 * Gets the fields and entities with missing alt attributes.
 *
 * @return array
 *   Keyed array of fields and entities with missing alt attributes.
 *   First set of array keys are the field machine name used. Second key is the
 *   entity type. Then 3rd key is the entity id. The value of the 3rd key is the
 *   entity to be used.
 *   Ex. array('field_image' => array('node' => array(10 => [Node Object])))
 */
function stanford_alt_check_get_images() {
  $image_fields = array();
  // Get all image type fields.
  foreach (field_info_fields() as $field) {
    if ($field['type'] == 'image') {
      $image_fields[] = $field;
    }
  }

  $return_data = array();
  // Gets all entities where the image alt attribute is missing.
  foreach ($image_fields as $field) {
    $query = new EntityFieldQuery();
    $query->fieldCondition($field['field_name'], 'alt', '', '=');
    $results = $query->execute();
    foreach ($results as $type => $result_entities) {
      $return_data[$field['field_name']][$type] = entity_load($type, array_keys($result_entities));
    }
  }
  return $return_data;
}

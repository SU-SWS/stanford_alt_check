<?php

/**
 * @file
 * Stanford Alt Check Upload Form.
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
  return $form;
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

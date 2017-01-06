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
function stanford_alt_check_image_uploads($form, &$form_state) {
  drupal_set_title(t("Image Uploads"));
  $form = array();
  $form['#tree'] = TRUE;

  $image_entities = stanford_alt_check_upload_entities();
  foreach ($image_entities as $field => $types) {
    foreach ($types as $type => $bundles) {
      foreach ($bundles as $bundle => $entities) {
        foreach ($entities as $id => $entity) {
          if ($images = field_get_items($type, $entity, $field)) {
            foreach ($images as $image) {

              // Alt is populated.
              if ($image['alt']) {
                continue;
              }

              // Display the image for quick reference.
              $image_url = image_style_url('thumbnail', $image['uri']);
              $image_full_url = file_create_url($image['uri']);
              $form['images'][$field][$type][$id][$image['fid']]['image'] = array(
                '#markup' => "<a href='$image_full_url' target='_blank'><img src='$image_url' /></a>",
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

              $t = $type;
              $e = $entity;

              // Field Collection entities: Change $t & $e to host entity.
              if (method_exists($entity, 'hostEntity')) {
                $t = $entity->hostEntityType();
                $e = $entity->hostEntity();
              }

              $title = $e->title;
              // Beans use the label property as the identifier.
              if ($t == 'bean') {
                $title = $e->label;
              }

              if (strlen($title) > 50) {
                $title = substr($title, 0, 50) . "...";
              }

              $entity_uri = entity_uri($t, $e);
              $form['images'][$field][$type][$id][$image['fid']]['entity_uri'] = array(
                '#markup' => l(t($title), $entity_uri['path']),
              );
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
function stanford_alt_check_image_uploads_submit($form, &$form_state) {
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

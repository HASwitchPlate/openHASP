---
title: "Quick-Start Guide"
permalink: /projects/hasp-lvgl/quick-start-guide/
excerpt: "How to quickly install and setup Minimal Mistakes for use with GitHub Pages."
last_modified_at: 2020-04-27
redirect_from:
  - /theme-setup/
toc: true
---

Minimal Mistakes has been developed as a [Gem-based theme](http://jekyllrb.com/docs/themes/) for easier use, and 100% compatible with GitHub Pages when used as a remote theme.

**If you enjoy this software, please consider [supporting me](https://www.paypal.me/netwize) for developing and maintaining it.**

[![Support via PayPal](https://cdn.jsdelivr.net/gh/twolfson/paypal-github-button@1.0.0/dist/button.svg)](https://www.paypal.me/mmistakes)

## Installing the theme

If you're running Jekyll v3.5+ and self-hosting you can quickly install the theme as a Ruby gem.

[^structure]: See [**Structure** page]({{ "/docs/structure/" | relative_url }}) for a list of theme files and what they do.

**ProTip:** Be sure to remove `/docs` and `/test` if you forked Minimal Mistakes. These folders contain documentation and test pages for the theme and you probably don't want them littering up your repo.
{: .notice--info}

**Note:** The theme uses the [jekyll-include-cache](https://github.com/benbalter/jekyll-include-cache) plugin which will need to be installed in your `Gemfile` and added to the `plugins` array of `_config.yml`. Otherwise you'll throw `Unknown tag 'include_cached'` errors at build.
{: .notice--warning}



### Firmware

Remote themes are similar to Gem-based themes, but do not require `Gemfile` changes or whitelisting making them ideal for sites hosted with GitHub Pages.

To install as a remote theme:

1. Create/replace the contents of your `Gemfile` with the following:

   ```ruby
   source "https://rubygems.org"

   gem "github-pages", group: :jekyll_plugins
   ```

2. Add `jekyll-include-cache` to the `plugins` array of your `_config.yml`.

3. Fetch and update bundled gems by running the following [Bundler](http://bundler.io/) command:

   ```bash
   bundle
   ```

4. Add `remote_theme: "mmistakes/minimal-mistakes@4.19.2"` to your `_config.yml` file. Remove any other `theme:` or `remote_theme:` entry.

You may also optionally specify a branch, [tag](https://github.com/mmistakes/minimal-mistakes/tags), or commit to use by appending an @ and the Git ref (e.g., `mmistakes/minimal-mistakes@4.9.0` or `mmistakes/minimal-mistakes@bbf3cbc5fd64a3e1885f3f99eb90ba92af84063d`). This is useful when rolling back to older versions of the theme. If you don't specify a Git ref, the latest on `master` will be used.

**Looking for an example?** Use the [Minimal Mistakes remote theme starter](https://github.com/mmistakes/mm-github-pages-starter/generate) for the quickest method of getting a GitHub Pages hosted site up and running. Generate a new repository from the starter, replace sample content with your own, and configure as needed.
{: .notice--info}

---

**Note:** Your Jekyll site should be viewable immediately at <http://USERNAME.github.io>. If it's not, you can force a rebuild by **Customizing Your Site** (see below for more details).
{: .notice--warning}

If you're hosting several Jekyll based sites under the same GitHub username you will have to use Project Pages instead of User Pages. Essentially you rename the repo to something other than **USERNAME.github.io** and create a `gh-pages` branch off of `master`. For more details on how to set things up check [GitHub's documentation](https://help.github.com/articles/user-organization-and-project-pages/).

<figure>
  <img src="{{ '/assets/images/mm-gh-pages.gif' | relative_url }}" alt="creating a new branch on GitHub">
</figure>

You can also install the theme by copying all of the theme files[^structure] into your project.

To do so fork the [Minimal Mistakes theme](https://github.com/mmistakes/minimal-mistakes/fork), then rename the repo to **USERNAME.github.io** --- replacing **USERNAME** with your GitHub username.

<figure>
  <img src="{{ '/assets/images/mm-theme-fork-repo.png' | relative_url }}" alt="fork Minimal Mistakes">
</figure>

**GitHub Pages Alternatives:** Looking to host your site for free and install/update the theme painlessly? [Netlify][netlify-jekyll], [GitLab Pages][gitlab-jekyll], and [Continuous Integration (CI) services][ci-jekyll] have you covered. In most cases all you need to do is connect your repository to them, create a simple configuration file, and install the theme following the [Ruby Gem Method](#ruby-gem-method) above.
{: .notice--info}

[netlify-jekyll]: https://www.netlify.com/blog/2015/10/28/a-step-by-step-guide-jekyll-3.0-on-netlify/
[gitlab-jekyll]: https://about.gitlab.com/2016/04/07/gitlab-pages-setup/
[ci-jekyll]: https://jekyllrb.com/docs/continuous-integration/

### Minimal Configuration

You need to setup network connectivity to your network and connect to an MQTT broker service.

## Home Automation

There are some examples available to help you integrate HASP into your Home Automation setup:
- Home Assistant
- OpenHAB
- Node-RED
- Tasmota Slave

**ProTip:** The source code and content files for this site can be found in the [`/docs` folder](https://github.com/mmistakes/minimal-mistakes/tree/master/docs) if you want to copy or learn from them.
{: .notice--info}


import os
import yaml
Import("env")

# 1. Resolve paths
project_dir = env.get("PROJECT_DIR")
src_dir = os.path.join(project_dir, "src")
manifest_path = os.path.join(src_dir, "idf_component.yml")

# 2. Extract configuration from platformio.ini
current_env = env.get("PIOENV")
# Get the specific env project configuration block
config = env.GetProjectConfig()
raw_dependencies = config.get(f"env:{current_env}", "custom_bsp_dependencies", None)

print(f"\n--- [Dynamic BSP Resolver] Env: {current_env} ---")

if raw_dependencies:
    # 3. Clean and parse the multiline configuration string
    lines = [line.strip() for line in raw_dependencies.strip().split("\n") if line.strip()]
    
    dependencies_dict = {}
    for line in lines:
        if ":" in line:
            package, version = line.split(":", 1)
            # Remove enclosing quotes if present in ini
            dependencies_dict[package.strip()] = version.strip().strip('"').strip("'")
    
    # 4. Construct the standard ESP-IDF Component structure
    manifest_data = {
        "dependencies": dependencies_dict
    }

    # 5. Write the manifest file cleanly
    print(f"--> Generating dynamic manifest with {len(dependencies_dict)} dependencies...")
    os.makedirs(src_dir, exist_ok=True)
    # with open(manifest_path, "w") as f:
    #   yaml.safe_dump(manifest_data, f, default_flow_style=False, sort_keys=False)
        
    # 6. Flush previous lock files to force an immediate CMake re-evaluation
    lock_file = os.path.join(project_dir, "dependencies.lock")
    if os.path.exists(lock_file):
        os.remove(lock_file)
        
else:
    print("--> No 'custom_bsp_dependencies' defined for this environment.")
    # Optional: Delete any old manifest so the build defaults back to base components
    if os.path.exists(manifest_path):
        os.remove(manifest_path)
